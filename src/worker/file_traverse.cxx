#include <emscripten.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include <array>
#include <memory>
#include <assert.h>
#include "wapi.h"
#include "commondef.h"
#include <extern/stb_image.h>

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

	const char* port = EM_SERV_ASSET_PORT;

	EM_ASM_({
		self.beginFetch($0, $1, $2, $3);
	}, args->proxy, args->data, args->size,
		port );
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
		
		const char* port = EM_SERV_ASSET_PORT;
	
		char urlString[ 36 ];
		memset( urlString, 0, sizeof( urlString ) );
		
		strncat( urlString, "http://localhost:", 17 );
		strncat( urlString, port, 4 );
		strncat( urlString, "/js/fetch.js", 12 );

		emscripten_async_wget_data( urlString, ( void* ) gTmpArgs.get(), 
				OnLoad, OnError );

		gInitialized = true;
		return false;
	}

	return true;
}

static int Ceil( int n )
{
	return n + 1;
}

struct file_t
{
	FILE* ptr;

	std::vector< unsigned char > readBuff;

	file_t( const std::string& path )
		: ptr( nullptr )
	{
		Open( path );
	}

	operator bool ( void ) const
	{
		return !!ptr;
	}

	void Open( const std::string& path )
	{
		if ( ptr )
		{
			fclose( ptr );
		}

		ptr = fopen( path.c_str(), "rb" );

		printf( "Attempting fopen for \'%s\'...\n", path.c_str() );
	}

	bool ReadImage( void )
	{
		int width, height, bpp; // bpp is in bytes...

		if ( !ptr )
		{
			puts( "ERROR: file could not be opened" );
			return false;
		}

		stbi_uc* buf =
			stbi_load_from_file( ptr, &width, &height, &bpp, STBI_default );

		if ( !buf )
		{
			puts( "ERROR: STBI rejected the image file" );
			return false;
		}

		printf( "Image Read successful:\n"\
	 			"width: %i, height: %i, bpp: %i\n",
				width, height, bpp );

		int target = width * height * bpp;
		int original = target;

		// Next level bit h4x so 1337 omg w0w
		// (we just want a 32-bit word fetch; in all honesty this may or may
		// not improve things since it's in a VM but w/e)
		if ( ( ( target >> 2 ) << 2 ) != target ) // same as target % 4 != 0
		{
			int next = target & ( ~3 );
			next += 4;
			target = next;
		}

		readBuff.resize( target + 8, 0 );
		memcpy( &readBuff[ 8 ], buf, original );

		// There's no way that we'll need more than 16 bits for each dimension.
		// Remaining 3 bytes are for padding.
		readBuff[ 0 ] = ( unsigned char )( width & 0xFF );
		readBuff[ 1 ] = ( unsigned char )( ( width >> 8 ) & 0xFF );
		readBuff[ 2 ] = ( unsigned char )( height & 0xFF );
		readBuff[ 3 ] = ( unsigned char )( ( height >> 8 ) & 0xFF );
		readBuff[ 4 ] = ( unsigned char )( bpp );

		stbi_image_free( buf );

		return true;
	}

	bool Read( size_t offset, size_t size )
	{
		if ( !ptr )
		{
			return false;
		}
		
		readBuff.resize( size, 0 );
		fseek( ptr, offset, SEEK_SET );
		fread( &readBuff[ 0 ], size, 1, ptr );
		readBuff.clear();

		return true;
	}

	bool Read( void )
	{
		if ( !ptr )
		{
			return false;
		}	

		fseek( ptr, 0, SEEK_END );
		return Read( 0, ftell( ptr ) );			
	}

	void Send( void ) const
	{
		emscripten_worker_respond( ( char* ) &readBuff[ 0 ], readBuff.size() );
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

static INLINE bool GetExt( const std::string& name, std::string& outExt )
{
	size_t index = name.find_last_of( '.' );

	if ( index == std::string::npos )
	{
		return false;
	}

	outExt = name.substr( index, name.size() - index );

	return true;
}

static INLINE std::string StripExt( const std::string& name )
{
	size_t index = name.find_last_of( '.' );

	if ( index == std::string::npos )
	{
		return name;
	}

	return name.substr( 0, index );
}

static std::string ReplaceExt( const std::string& path, const std::string& ext )
{
	std::string f( StripExt( path ) );
	f += ext;
	return f;
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

	gFIOChain.release();

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

static void ReadImage_Proxy( char* path, int size )
{
	std::string full( FullPath( path ) );

	gFIOChain.reset( new file_t( full ) );

	if ( !( *gFIOChain ) )
	{
		std::array< std::string, 3 > candidates =
		{
			".jpg", ".tga", ".jpeg"
		};

		std::string firstExt;
		bool hasExt = GetExt( full, firstExt );

		for ( size_t i = 0; i < candidates.size(); ++i )
		{
			if ( hasExt && firstExt == candidates[ i ] )
			{
				continue;
			}

			full = ReplaceExt( full, candidates[ i ] );
			gFIOChain->Open( full );

			if ( *gFIOChain )
			{
				break;
			}
		}

		if ( !( *gFIOChain ) )
		{
			FailOpen( path );
			return;
		}
	}

	if ( !gFIOChain->ReadImage() )
	{
		FailOpen( path );
		return;
	}

	gFIOChain->Send();
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

void ReadImage( char* path, int size )
{
	puts( "Worker: ReadImage entering" );

	if ( InitSystem( ReadImage_Proxy, path, size ) )
	{
		ReadImage_Proxy( path, size );
	}
}

} // extern "C"
