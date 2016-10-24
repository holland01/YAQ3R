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

#define AL_STRING_DELIM '|'

void TestFile( unsigned char* path )
{
	char* strpath = ( char* )path;

	emscripten_worker_respond_provisionally( strpath, strlen( strpath ) );
}

typedef void ( *callback_t )( char* data, int size );

static bool gInitialized = false;

struct charBuff_t
{
	char* data;
	int size;

	charBuff_t( const char* data_, int size_ )
		:
			data( nullptr ),
			size( size_ )
	{
		if ( data_ )
		{
			data = new char[ size + 1  ];
			memset( data, 0, size + 1 );
			memcpy( data, data_, size );
		}
	}

	~charBuff_t( void )
	{
		if ( data )
		{
			delete[] data;
		}
	}
};

struct asyncArgs_t
{
	callback_t proxy; // what to call after the asynchronous fetch is successful
	charBuff_t buff;

	asyncArgs_t( callback_t proxy_, char* data, int size )
		: proxy( proxy_ ),
		  buff( data, size )
	{}
};

static std::unique_ptr< asyncArgs_t > gTmpArgs( nullptr );

static void InitSystem_OnError( void* arg )
{
	( void )arg;
	puts( "emscripten_async_wget_data inject fetch failed" );
}

static void InitSystem_OnLoad( void* arg, void* data, int size )
{
	asyncArgs_t* args = ( asyncArgs_t* )arg;

	std::vector< char > copyData( size + 1, 0 );
	memcpy( &copyData[ 0 ], data, size );
	emscripten_run_script( &copyData[ 0 ] );

	args->proxy( args->buff.data, args->buff.size );
}

static bool InitSystem( callback_t proxy, char* data, int size )
{
	if ( !gInitialized )
	{
		gTmpArgs.reset( new asyncArgs_t( proxy, data, size ) );

		char urlString[ 36 ];
		memset( urlString, 0, sizeof( urlString ) );

		const char* port = EM_SERV_ASSET_PORT;

		strncat( urlString, "http://localhost:", 17 );
		strncat( urlString, port, 4 );
		strncat( urlString, "/js/fetch.js", 12 );

		emscripten_async_wget_data( urlString, ( void* ) gTmpArgs.get(),
				InitSystem_OnLoad, InitSystem_OnError );

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
		readBuff.clear();

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

		// Keep things aligned, if only for pedantry
		if ( ( ( target >> 2 ) << 2 ) != target )
		{
			int next = target & ( ~3 );
			next += 4;
			target = next;
		}

		readBuff.resize( target + 8, 0 );
		memcpy( &readBuff[ 8 ], buf, original );

		// There's no way that we'll need more
		// Than 16 bits for each dimension.
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

		readBuff.clear();

		if ( size )
		{
			readBuff.resize( size, 0 );
			fseek( ptr, offset, SEEK_SET );
			fread( &readBuff[ 0 ], size, 1, ptr );
		}

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
		emscripten_worker_respond( ( char* ) &readBuff[ 0 ],
			readBuff.size() );
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

static INLINE std::string FullPath( const char* path, size_t pathLen )
{
	const char* croot = "/working";

	std::string root( croot );

	std::string strPath = "/";
	strPath.append( path, pathLen );

	root.append( strPath );

	printf( "Path Received: %s\n", root.c_str() );

	return root;
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

static std::string ReplaceExt( const std::string& path,
	const std::string& ext )
{
	std::string f( StripExt( path ) );
	f += ext;
	return f;
}

static INLINE void FailOpen( const char* path, size_t pathLen )
{
	uint32_t m = WAPI_FALSE;
	std::string strPath( path, pathLen );
	printf( "fopen for \'%s\' failed\n", strPath.c_str() );
	emscripten_worker_respond( ( char* ) &m, sizeof( m ) );
}

static INLINE bool SplitDataWithBundle( std::string& bundleName,
	std::vector< char >& chopData, char* data, int size )
{
	int i;
	for ( i = 0; i < size; ++i ) {
		if ( data[i] == AL_STRING_DELIM ) {
			bundleName = std::string( data, i );
			printf( "Bundle Name Found: %s\n", bundleName.c_str() );

			chopData.resize( size - i + 1, 0 );
			memcpy( &chopData[ 0 ], data + i + 1, size - i );
			return true;
		}
	}

	return false;
}

static void SendFile_OnLoad( char* path, int size )
{
	puts("SendFile_OnLoad reached.");

	const std::string& fp = FullPath( path, size );

	gFIOChain.reset( new file_t( fp ) );

	if ( *gFIOChain )
	{
		gFIOChain->Send();
	}
	else
	{
		FailOpen( fp.c_str(), size );
	}
}

static INLINE bool SplitBundlePath( std::string& bundleName,
		std::vector< char >& remData, char* data, int size )
{
	if ( !SplitDataWithBundle( bundleName, remData, data, size ) )
	{
		FailOpen( data, size );
		return false;
	}
	return true;
}

static void ReadFile_Proxy( char* data, int size )
{
	std::string bundleName;
	std::vector< char > remData;
	if ( !SplitBundlePath( bundleName, remData, data, size ) ) return;

	const char* port = EM_SERV_ASSET_PORT;

	printf( "Remaining Data: %s\n", &remData[ 0 ] );

	EM_ASM_ARGS( {
		self.fetchBundleAsync($0, $1, $2, $3, $4);
	}, bundleName.c_str(), SendFile_OnLoad,
	   &remData[ 0 ], remData.size() - 1, // don't include null term
	   port );
}

static void SendShader_OnLoad( char* path, int size )
{
	if ( !path && !size )
	{
		puts( "Last shader path found." );
		emscripten_worker_respond( nullptr, 0 );
		return;
	}

	std::string strPath( path, size );

	gFIOChain.reset( new file_t( strPath ) );

	if ( !gFIOChain->Read() )
	{
		FailOpen( path, size );
		return;
	}

	std::vector< char > buffer( gFIOChain->readBuff.size() + size + 1, 0 );

	memcpy( &buffer[ 0 ], path, size );
	memcpy( &buffer[ size + 1 ], &gFIOChain->readBuff[ 0 ],
			gFIOChain->readBuff.size() );

	buffer[ size ] = AL_STRING_DELIM;

	gFIOChain.release();

	emscripten_worker_respond_provisionally( &buffer[ 0 ],
		buffer.size() );
}

static void TraverseDirectory_Read( char* dir, int size )
{
	if ( !dir )
	{
		FailOpen( dir, size );
		return;
	}

	std::string mountDir( FullPath( dir, size ) );

	char error[ 256 ];
	memset( error, 0, sizeof( error ) );

	int code = EM_ASM_ARGS({
			try {
				return self.walkFileDirectory($0, $1, $2);
			} catch (e) {
				console.log(e.message);
				return 0;
			}
		},
		mountDir.c_str(),
		SendShader_OnLoad,
		error
	);

	if ( !code )
	{
		printf( "Failed to traverse \'%s\'\n", dir );
	}

	puts( "TraverseDirectory_Read finished" );
}

static void TraverseDirectory_Proxy( char* data, int size )
{
	std::string bundleName;
	std::vector< char > remData;
	if ( !SplitBundlePath( bundleName, remData, data, size ) ) return;

	const char* port = EM_SERV_ASSET_PORT;

	printf( "Bundle Name: %s\n Rem Data: %s\n", bundleName.c_str(),
		&remData[ 0 ] );

	EM_ASM_ARGS({
			self.fetchBundleAsync($0, $1, $2, $3);
		},
		bundleName.c_str(),
		TraverseDirectory_Read,
		&remData[ 0 ],
		remData.size() - 1, // don't include null term
		port
	);
}

static void ReadImage_Proxy( char* path, int size )
{
	std::string strPath( path, size );
	std::string full( FullPath( path, size ) );

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
			FailOpen( path, size  );
			return;
		}
	}

	if ( !gFIOChain->ReadImage() )
	{
		FailOpen( path, size );
		return;
	}

	gFIOChain->Send();
}

void UnmountPackages_Proxy( char* data, int size )
{
	EM_ASM({
		self.unmountPackages();
	});

	uint32_t success = WAPI_TRUE;
	emscripten_worker_respond( ( char* ) &success, sizeof( success ) );
}

void MountPackage_Proxy( char* data, int size )
{
	const char* port = EM_SERV_ASSET_PORT;

	EM_ASM_ARGS({
			self.fetchBundleAsync($0, $1, $2, $3);
		},
		data,
		emscripten_worker_respond,
		0,
		0,
		port
	);
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

	wApiChunkInfo_t* cmd = ( wApiChunkInfo_t* )bcmd;

	printf( "Received - offset: " F_SIZE_T ", size: " F_SIZE_T "\n",
		cmd->offset, cmd->size );

	gFIOChain->Read( cmd->offset, cmd->size );
	emscripten_worker_respond( ( char* )&gFIOChain->readBuff[ 0 ], cmd->size );
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

void MountPackage( char* path, int size )
{
	if ( InitSystem( MountPackage_Proxy, path, size ) )
	{
		MountPackage_Proxy( path, size );
	}
}

void UnmountPackages( char* data, int size )
{

	if ( InitSystem( UnmountPackages_Proxy, nullptr, 0 ) )
	{
		UnmountPackages_Proxy( nullptr, 0 );
	}
}

} // extern "C"
