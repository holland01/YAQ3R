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
		self.beginFetch($0, $1, $2 );
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
		fread( &readBuff[ 0 ], size, 1, ptr );

		return true;
	}

	bool Read( void )
	{
		if ( !ptr )
		{
			return false;
		}

		readBuff.clear();

		fseek( ptr, 0, SEEK_END );
		readBuff.resize( ftell( ptr ), 0 );
		rewind( ptr );

		fread( &readBuff[ 0 ], readBuff.size(), 1, ptr );

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

static INLINE std::string FullPath( const char* path )
{
	std::string root( "/working" );

	if ( path[ 0 ] != '/' )
	{
		root.append( 1, '/' );
	}

	std::string absp( root );
	absp.append( path );

	printf( "Path Received: %s\n", absp.c_str() );

	return absp;
}

static INLINE void FailOpen( const char* path )
{
	uint32_t m = WAPI_FALSE;
	printf( "fopen for \'%s\' failed\n", path );
	emscripten_worker_respond( ( char* ) &m, sizeof( m ) );
}

static void ReadFile_Proxy( char* path, int size )
{
	gFIOChain.reset( new file_t( FullPath( path ) ) );

	if ( *gFIOChain )
	{
		uint32_t m;
		m = WAPI_TRUE;
		emscripten_worker_respond( ( char* ) &m, sizeof( m ) );
	}
	else
	{
		FailOpen( path );
	}
}

static void TraverseDirectory_Read( char* path, int size )
{
	if ( !path )
	{
		emscripten_worker_respond( nullptr, 0 );
		return;
	}

	gFIOChain.reset( new file_t( std::string( path ) ) );

	if ( !gFIOChain->Read() )
	{
		FailOpen( path );
		return;
	}

	// 'size' and readBuff.size() will each already
	// include space for a null term, so we can expend
	// one of them for a delimiter
	std::vector< char > buffer( gFIOChain->readBuff.size() + size, 0 );
	memcpy( &buffer[ 0 ], path, size );
	memcpy( &buffer[ size ], &gFIOChain->readBuff[ 0 ],
		gFIOChain->readBuff.size() );
	buffer[ size ] = '|'; // <- delim

	emscripten_worker_respond_provisionally( &buffer[ 0 ],
	 	buffer.size() );
}

static void TraverseDirectory_Proxy( char* dir, int size )
{
	std::string mountDir( FullPath( dir ) );
	char error[ 256 ];
	memset( error, 0, sizeof( error ) );
	int code = EM_ASM_ARGS({
		try {
			return self.walkFileDirectory($0, $1, $2);
		} catch (e) {
			console.log(e.message);
			return 0;
		}
	}, mountDir.c_str(), TraverseDirectory_Read, error );

	if ( !code )
	{
		printf( "Failed to traverse \'%s\'\n", dir );
	}
}

extern "C" {

void ReadFile_Begin( char* path, int size )
{
	puts( "Worker: ReadFile_Begin entering" );

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

void TraverseDirectory( char* dir, int size )
{
	puts( "Worker: TraverseDirectory entering" );

	if ( InitSystem( TraverseDirectory_Proxy, dir, size ) )
	{
		TraverseDirectory_Proxy( dir, size );
	}
}

} // extern "C"
