#include <emscripten.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include <memory>
#include <assert.h>
#include "wapi.h"
#include "../commondef.h"

void TestFile( unsigned char* path )
{
	char* strpath = ( char* )path;

	emscripten_worker_respond_provisionally( strpath, strlen( strpath ) );
}

typedef void ( *callback_t )( char* data, int size );

static bool gInitialized = false;

struct asyncArgs_t
{
	callback_t proxy; // what to call after the asynchronous fetch is successful
	char* data;
	int size; // in bytes

	asyncArgs_t( callback_t proxy_, char* data_, int size_ )
		: proxy( proxy_ ),
		  data( data_ ),
		  size( size_ )
	{}

	~asyncArgs_t( void )
	{
		if ( data )
		{
			delete data;
		}
	}
};

static std::unique_ptr< asyncArgs_t > gTmpArgs( nullptr );

static void OnError( void* arg )
{
	( void )arg;
	puts( "emscripten_async_wget_data inject fetch failed" );
}

static void OnLoad( void* arg, void* data, int size )
{
	asyncArgs_t* args = ( asyncArgs_t* )arg;

	std::vector< char > copyData( size + 1, 0 );
	memcpy( &copyData[ 0 ], data, size );
	emscripten_run_script( &copyData[ 0 ] );

	EM_ASM_({
		self.beginFetch($0, $1, $2);
	}, args->proxy, args->data, args->size );
}

static bool InitSystem( callback_t proxy, char* data, int size )
{
	if ( !gInitialized )
	{
		// see if we can figure out which bundle holds our data
		char* dup = new char[ size + 1 ]();
		memset( dup, 0, size + 1 );
		memcpy( dup, data, size );
		gTmpArgs.reset( new asyncArgs_t( proxy, dup, size ) );
		emscripten_async_wget_data( "http://localhost:6931/js/fetch.js",
			 ( void* ) gTmpArgs.get(), OnLoad, OnError );

		gInitialized = true;
		return false;
	}

	return true;
}

struct file_t
{
	FILE* ptr;

	std::vector< unsigned char > readBuff;

	file_t( const std::string& path )
	:	ptr( fopen( path.c_str(), "rb" ) )
	{
	}

	operator bool ( void ) const
	{
		return !!ptr;
	}

	bool Read( size_t offset, size_t size )
	{
		if ( !ptr )
		{
			return false;
		}

		if ( !readBuff.empty() )
		{
			memset( &readBuff[ 0 ], 0, readBuff.size() * sizeof( unsigned char ) );
		}

		if ( readBuff.size() < size )
		{
			readBuff.resize( size, 0 );
		}

		fseek( ptr, offset, SEEK_SET );
		fread( &readBuff[ 0 ], 1, size, ptr );

		return true;
	}

	~file_t( void )
	{
		if ( ptr )
		{
			fclose( ptr );
		}
	}
};

static std::unique_ptr< file_t > gFIOChain( nullptr );

static void ReadFile_Proxy( char* path, int size )
{
	std::string root( "/working" );

	if ( path[ 0 ] != '/' )
	{
		root.append( 1, '/' );
	}

	std::string absp( root );
	absp.append( path );

	printf( "Path Received: %s\n", absp.c_str() );

	gFIOChain.reset( new file_t( absp ) );

	uint32_t m;
	if ( *gFIOChain )
	{
		m = WAPI_TRUE;
	}
	else
	{
		printf( "fopen for \'%s\' failed\n", path );
		m = WAPI_FALSE;
	}

	emscripten_worker_respond( ( char* ) &m, sizeof( m ) );
}

extern "C" {

void ReadFile_Begin( char* path, int size )
{
	puts( "Worker: ReadFile entering" );

	if ( InitSystem( ReadFile_Proxy, path, size ) )
	{
		ReadFile_Proxy( path, size );
	}
}

void ReadFile_Chunk( char* bcmd, int size )
{
	if ( !gFIOChain || !( *gFIOChain ) )
	{
		puts( "No file initialized..." );
		emscripten_worker_respond( nullptr, 0 );
		return;
	}

	wApiChunkInfo_t* cmd =  ( wApiChunkInfo_t* )bcmd;

	if ( gFIOChain->Read( cmd->offset, cmd->size ) )
	{
		emscripten_worker_respond( ( char* )&gFIOChain->readBuff[ 0 ], cmd->size );
	}
	else
	{
		uint32_t m = WAPI_FALSE;
		emscripten_worker_respond( ( char* ) &m, sizeof( m ) );
	}
}

void Traverse( char* directory, int size )
{
	puts( "Worker: Traverse entering" );

	( void )size;

	if( 0 )
	{
		InitSystem( nullptr, directory, size );
	}
	char errorMsg[ 128 ];
	memset( errorMsg, 0, sizeof( errorMsg ) );

	int ret = EM_ASM_ARGS(
		console.log('Does this work lol: ', self.walkFileDirectory);
		try {
			return self.walkFileDirectory($0, $1, $2);
		} catch (e) {
			console.log(e);
			throw e;
		}
		return 0;
	, directory, TestFile, errorMsg );

	if ( !ret )
		printf( "[WORKER ERROR]: %s\n", errorMsg );

	emscripten_worker_respond( NULL, 0 );
}

} // extern "C"