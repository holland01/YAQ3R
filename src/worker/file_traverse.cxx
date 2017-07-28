#include <emscripten.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <sstream>
#include <vector>
#include <array>
#include <memory>
#include <assert.h>
#include <stdarg.h>
#include "wapi.h"
#include "commondef.h"
#include <extern/stb_image.h>

//#define DEBUG
#define CONTENT_PIPELINE_IO

#define AL_STRING_DELIM '|'

void TestFile( unsigned char* path )
{
	char* strpath = ( char* )path;

	emscripten_worker_respond_provisionally( strpath, strlen( strpath ) );
}

static void O_Log( const char* fmt, ... )
{
#if defined( DEBUG ) && defined( CONTENT_PIPELINE_IO )
	va_list arg;

	va_start( arg, fmt );
	vfprintf( stdout, fmt, arg );
	fputs( "\n", stdout );
	va_end( arg );
#else
	UNUSED( fmt );
#endif
}

static INLINE std::string FullPath( const char* path, size_t pathLen )
{
	const char* croot = "/working";
	std::string root( croot );

	std::string strPath = "/";
	strPath.append( path, pathLen );
	root.append( strPath );

	return root;
}

static INLINE bool GetExt( const std::string& name, std::string& outExt )
{
	size_t index = name.find_last_of( '.' );

	if ( index == std::string::npos )
	{
		outExt = "";
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

	f.append( ext );

	return f;
}

static INLINE void FailOpen( const char* path, size_t pathLen )
{
	uint32_t m = WAPI_FALSE;
	std::string strPath( path, pathLen );
	O_Log( "fopen for \'%s\' failed\n", strPath.c_str() );
	emscripten_worker_respond( ( char* ) &m, sizeof( m ) );
}

static INLINE bool SplitDataWithBundle( std::string& bundleName,
	std::vector< char >& chopData, char* data, int size )
{
	for ( int i = 0; i < size; ++i )
	{
		if ( data[i] == AL_STRING_DELIM )
		{
			bundleName = std::string( data, i );
			O_Log( "Bundle Name Found: %s\n", bundleName.c_str() );

			chopData.resize( size - i + 1, 0 );
			memcpy( &chopData[ 0 ], data + i + 1, size - i );
			return true;
		}
	}

	return false;
}

static void ReadImage_Proxy( const char* path, int size );

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
	}

	bool ReadImage( void )
	{
		readBuff.clear();

		int width, height, bpp; // bpp is in bytes...

		if ( !ptr )
		{
			O_Log( "%s", "ERROR: file could not be opened" );
			return false;
		}

		stbi_uc* buf =
			stbi_load_from_file( ptr, &width, &height, &bpp, STBI_default );

		if ( !buf )
		{
			O_Log( "%s",  "ERROR: STBI rejected the image file" );
			return false;
		}

		size_t size = width * height * bpp;
		size_t cap = size + 8;

		readBuff.resize( cap, 0 );
		memcpy( &readBuff[ 8 ], buf, size );

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
			readBuff.resize( size + 1, 0 );
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

	void Send( void )
	{
		if ( readBuff.empty() )
		{
			char truth[ 4 ] = { 0x7F, 0x7F, 0x7F, 0x7F };
			emscripten_worker_respond( &truth[ 0 ], sizeof( truth ) );
		}
		else
		{
			uint8_t checksum = WAPI_CalcCheckSum(
				( char* ) &readBuff[ 0 ],
		 		readBuff.size() - 1
			);

			readBuff[ readBuff.size() - 1 ] = ( char ) checksum;

			emscripten_worker_respond(
				( char* ) &readBuff[ 0 ],
				readBuff.size()
			);
		}
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


class Bundle
{
	enum
	{
		BUNDLE_PATH_SIZE = 64
	};

	struct bufferMeta_t
	{
		int32_t sliceOffset;
		int32_t sliceSize;
		int32_t slicesLeft;
		int32_t metaByteLen;
	};

	struct bundleMeta_t
	{
		int32_t startOffset;
		int32_t endOffset;
		char filepath[ BUNDLE_PATH_SIZE ];
	};

	const char* bundle = nullptr;

	size_t numMetaEntries = 0;
	size_t iterator = 0;

	const bufferMeta_t* bufferInfo = nullptr; // is not a buffer of memory: just one struct "instance"
	const bundleMeta_t* metadata = nullptr; // *is* a buffer of memory, there's going to be X count of entries.

public:
	size_t GetIterator( void ) const { return iterator; }

	size_t GetNumFiles( void ) const { return numMetaEntries; }

	void Clear( bool freeBuffer = true )
	{
		iterator = 0;
		bundle = nullptr;
		bufferInfo = nullptr;
		metadata = nullptr;
		numMetaEntries = 0;

		// If the AL.buffer.ptr is already freed then this will nop since
		// the free function makes sure that AL.buffer.ptr != 0
		// before proceeding. AL.buffer.ptr is set to 0
		// after each free.
		if ( freeBuffer )
		{
			EM_ASM({ self.clearBuffer(); });
		}
	}

	void PrintMetadata( void ) const
	{
#if defined( CONTENT_PIPELINE_IO ) && defined( DEBUG )
		std::stringstream out;
		for ( uint32_t i = 0; i < numMetaEntries; ++i )
		{
			out << "[" << i << "]:\n"
				<< "\tpath: " << metadata[ i ].filepath
				<< "\tstart: " << metadata[ i ].startOffset
				<< "\tend: " << metadata[ i ].endOffset
				<< "\n";
		}
		O_Log( "Entries:\n %s", out.str().c_str() );
#endif
	}

	// Three different structures: the bundle itself (in javascript is represented as blob),
	// the metadata for the bundle,
	// and the metadata for the buffer.
	// The order goes:
	// [0] -> bufferMeta_t: metadata for the buffer
	// [1] -> bundleMeta_t: metadata for the blob
	// [2] -> char*: the bundle
	void Load( const char* buffer, int size )
	{
		Clear( false );

		bufferInfo = ( const bufferMeta_t* ) &buffer[ 0 ];
		metadata = ( const bundleMeta_t* ) &buffer[ sizeof( *bufferInfo ) ];
		bundle = &buffer[ sizeof( *bufferInfo ) + bufferInfo->metaByteLen ];

		numMetaEntries = bufferInfo->metaByteLen / sizeof( *metadata );

		PrintMetadata();
	}

	const char* GetFile( uint32_t metadataIndex, int& outSize ) const
	{
		const bundleMeta_t& m = metadata[ metadataIndex ];
		outSize = static_cast< int >( m.endOffset - m.startOffset );
		return &bundle[ m.startOffset ];
	}

	int FindFile( const char* path ) const
	{
		size_t len = strlen( path );

		for ( uint32_t i = 0; i < numMetaEntries; ++i )
		{
			if ( strncmp( path, metadata[ i ].filepath, len ) == 0 )
			{
				return i;
			}
		}

		return -1;
	}

	const char* GetFile( const char* path, int& outSize ) const
	{
		int index = FindFile( path );

		if ( index >= 0 )
		{
			return GetFile( index, outSize );
		}

		return nullptr;
	}

	bool IsShader( void ) const
	{
		std::string filepath( metadata[ iterator ].filepath );
		std::string ext;
		GetExt( filepath, ext );

		return ext == ".shader";
	}

	std::vector<char> ReadImage( const char* filename )
	{
		std::vector<char> imgBuff;

		int width, height, bpp; // bpp is in bytes...

		int outSize;
		const char* ptr = GetFile( filename, outSize );

		if ( !ptr )
		{
			O_Log( "ERROR: Could not find image file: %s", filename );
			return imgBuff;
		}

		stbi_uc* buf = stbi_load_from_memory( ( const stbi_uc* ) ptr, outSize, &width, &height, &bpp, STBI_default );

		if ( !buf )
		{
			O_Log( "ERROR: STBI rejected the image file %s", filename );
			return imgBuff;
		}

		size_t size = width * height * bpp;
		size_t cap = size + 8;

		imgBuff.resize( cap, 0 );
		memcpy( &imgBuff[ 8 ], buf, size );

		// There's no way that we'll need more
		// Than 16 bits for each dimension.
		// Remaining 3 bytes are for padding.
		imgBuff[ 0 ] = ( unsigned char )( width & 0xFF );
		imgBuff[ 1 ] = ( unsigned char )( ( width >> 8 ) & 0xFF );
		imgBuff[ 2 ] = ( unsigned char )( height & 0xFF );
		imgBuff[ 3 ] = ( unsigned char )( ( height >> 8 ) & 0xFF );
		imgBuff[ 4 ] = ( unsigned char )( bpp );

		stbi_image_free( buf );

		return imgBuff;
	}

	// Stitches two buffers together,
	// inserting a '|' delimiter between both of them
	std::vector< char > MakeBuffer(
		const char* a, int sizeA,
		const char* b, int sizeB,
		bool addNull
	) const
	{
		std::vector< char > buffer( sizeA + sizeB + ( addNull? 2 : 1 ), 0 );
		buffer[ sizeA ] = AL_STRING_DELIM;

		memcpy( &buffer[ 0 ], a, sizeA );
		memcpy( &buffer[ sizeA + 1 ], b, sizeB );

		return buffer;
	}

	void SendFileProvisionally( int index ) const
	{
		if ( index == -1 )
		{
			return;
		}

		int size;

		const char* file = GetFile( index, size );

		std::vector< char > transfer( MakeBuffer(
			metadata[ index ].filepath,
			strlen( metadata[ index ].filepath ),
			file,
			size,
			true	// Add null, since shader parser uses c string functions on it
		) );

		emscripten_worker_respond_provisionally(
			&transfer[ 0 ], transfer.size() );
	}

	void SendEmptyProvisionally( void ) const
	{
		emscripten_worker_respond_provisionally( nullptr, 0 );
	}

	void FinishStream( void )
	{
		Clear();
		emscripten_worker_respond( nullptr, 0 );
	}

	void SendNextShader( void )
	{
		while ( !IsShader() )
		{
			iterator++;
		}

		if ( iterator < GetNumFiles() )
		{
			SendFileProvisionally( iterator );
			iterator++;
		}
	}

	void FailStream( void )
	{
		O_Log( "Could not stream image file: %s",
			metadata[ iterator ].filepath );
		FinishStream();
	}

	void SendNextImage( void )
	{
		if ( iterator < GetNumFiles() )
		{
			ReadImage_Proxy(
				metadata[ iterator ].filepath,
				strlen( metadata[ iterator ].filepath )
			);

			iterator++;
		}
	}

	~Bundle( void )
	{
		Clear();
	}
};

static std::unique_ptr< Bundle > gBundle( new Bundle() );

static void InitSystem_OnError( void* arg )
{
	( void )arg;
	O_Log( "%s", "emscripten_async_wget_data inject fetch failed" );
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

static void SendMapFile( char* path, int size )
{
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

static void ReadMapFile_Proxy( char* data, int size )
{
	std::string bundleName;
	std::vector< char > remData;

	if ( !SplitBundlePath( bundleName, remData, data, size ) )
		return;

	const char* port = EM_SERV_ASSET_PORT;

	EM_ASM_ARGS(
		{
			self.fetchBundleAsync($0, $1, $2, $3, $4);
		},
		bundleName.c_str(),
		SendMapFile,
		&remData[ 0 ],
		remData.size() - 1, // don't include null term
		port
	);
}

static void LoadShadersAndStream( char* data, int size )
{
	gBundle->Load( data, size );

	while ( gBundle->GetIterator() < gBundle->GetNumFiles() )
	{
		gBundle->SendNextShader();
	}

	gBundle->FinishStream();
}

static void ReadShaders_Proxy( char* data, int size )
{
	std::string bundleName;
	std::vector< char > remData;

	if ( !SplitBundlePath( bundleName, remData, data, size ) )
		return;

	const char* port = EM_SERV_ASSET_PORT;

	O_Log(
		"Bundle Name: %s\n Rem Data: %s\n",
		bundleName.c_str(),
		&remData[ 0 ]
	);

	EM_ASM_ARGS(
		{
			self.fetchBundleAsync($0, $1, $2, $3, $4, true);
		},
		bundleName.c_str(),
		LoadShadersAndStream,
		&remData[ 0 ],
		remData.size() - 1, // don't include null term
		port
	);
}

static void ReadImage_Proxy( const char* path, int size )
{
	std::string full( path );

	std::vector< char > imgBuff = gBundle->ReadImage( full.c_str() );

	if ( imgBuff.empty() )
	{
		std::array< std::string, 3 > candidates =
		{
			".jpg", ".tga", ".jpeg"
		};

		std::string firstExt;
		volatile bool hasExt = GetExt( full, firstExt );

		for ( size_t i = 0; i < candidates.size() && imgBuff.empty(); ++i )
		{
			if ( hasExt && firstExt == candidates[ i ] )
			{
				continue;
			}

			full = ReplaceExt( full, candidates[ i ] );

			imgBuff = gBundle->ReadImage( full.c_str() );
		}

		if ( imgBuff.empty() )
		{
			gBundle->SendEmptyProvisionally();
			return;
		}
	}

	emscripten_worker_respond_provisionally( &imgBuff[ 0 ], imgBuff.size() );
}

static void LoadImagesAndStream( char* buffer, int size )
{
	gBundle->Load( buffer, size );
	while ( gBundle->GetIterator() < gBundle->GetNumFiles() )
	{
		gBundle->SendNextImage();
	}
	gBundle->FinishStream();
}

void UnmountPackages_Proxy( char* data, int size )
{
	gFIOChain.reset();
	gBundle->Clear();
	EM_ASM({
		self.unmountPackages();
	});
	uint32_t success = WAPI_TRUE;
	emscripten_worker_respond( ( char* ) &success, sizeof( success ) );
}

void MountPackage_Proxy( char* path, int size )
{
	const char* port = EM_SERV_ASSET_PORT;

	// 'bundle' is misleading: this is really the null-terminator-to-be
	// of the bundle string
	char* bundle = strchr( path, '|' );
	const char* paths = nullptr;
	size_t pathslen = 0;

	O_Log( "ALL = %s", path );


	// Replace the first pipe (marking the end of the bundle path)
	// with a null byte to keep the rest of the filepaths separate
	// from the bundle.
	if ( bundle )
	{
		*bundle = '\0';
		paths = bundle + 1;
		pathslen = size - 1 - strlen( path ); // path = bundle
	}

	O_Log( "BUNDLE = %s; PATHS = %s", path, paths );


	EM_ASM_ARGS(
		{
			self.fetchBundleAsync($0, $1, $2, $3, $4, true);
		},
		path, // this is the bundle
		LoadImagesAndStream,
		paths,
		pathslen,
		port
	);
}

extern "C" {

void ReadMapFile_Begin( char* path, int size )
{
	O_Log( "%s",  "Worker: ReadMapFile_Begin entering" );

	if ( InitSystem( ReadMapFile_Proxy, path, size ) )
	{
		ReadMapFile_Proxy( path, size );
	}
}

void ReadMapFile_Chunk( char* bcmd, int size )
{
	if ( !gFIOChain || !( *gFIOChain ) )
	{
		O_Log( "%s",  "No file initialized..." );
		emscripten_worker_respond( nullptr, 0 );
		return;
	}

	wApiChunkInfo_t* cmd = ( wApiChunkInfo_t* )bcmd;

	O_Log(
		"Received - offset: " F_SIZE_T ", size: " F_SIZE_T "\n",
		cmd->offset,
		cmd->size
	);

	gFIOChain->Read( cmd->offset, cmd->size );
	gFIOChain->Send();
}

void ReadShaders( char* dir, int size )
{
	O_Log( "%s",  "Worker: ReadShaders entering" );

	if ( InitSystem( ReadShaders_Proxy, dir, size ) )
	{
		ReadShaders_Proxy( dir, size );
	}
}

void ReadImage( char* path, int size )
{
	UNUSED( path );
	UNUSED( size );
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
