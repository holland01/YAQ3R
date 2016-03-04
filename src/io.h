#pragma once

#include "common.h"

struct bspFace_t;
struct bspMeshVertex_t;
class Q3BspMap;

struct drawSurface_t;
struct gTextureHandle_t;
struct gTextureImage_t;
struct mapData_t;
struct shaderStage_t;

void LogWriteAtlasTexture( std::stringstream& sstream,
						   const gTextureHandle_t& texHandle,
						   const shaderStage_t* stage );

void LogBSPData( int bspDataType, void* data, int length );

void O_Log( const char* header, const char* priority, const char* fmt, ... );
void O_LogBuffer( const char* header, const char* priority, const char* fmt, ... );
void O_LogF( FILE* f, const char* header, const char* fmt, ... );
void MyDateTime( const char* format, char* outBuffer, int length );

float GetTimeSeconds( void );

void ExitOnGLError( int line, const char* glFunc, const char* callerFunc );

void InitSysLog( void );
void InitLogBSPData( Q3BspMap* map );

void KillSysLog( void );

enum fileCommand_t
{
	FILE_CONTINUE_TRAVERSAL = 1,
	FILE_STOP_TRAVERSAL = 0
};

using filedata_t = uint8_t*;

// A return value of true (1) means "keep iterating, unless we're at the end"; false will terminate the iteration
typedef int ( *fileSystemTraversalFn_t )( const filedata_t data );

void File_IterateDirTree( std::string directory, fileSystemTraversalFn_t callback );

// Returns true if a trailing slash is needed in the path, otherwise false.
// Either way, the default OS_PATH_SEPARATOR is thrown in outSlash,
// or an alternative separator if the string already contains it (i.e., we're on Windows and
// the path isn't using back slashes...)
bool NeedsTrailingSlash( const std::string& path, char& outSlash );

#ifdef __GNUC__
#	define _FUNC_NAME_ __func__
#	define _LINE_NUM_ __LINE__
#elif defined (_MSC_VER)
#	define _FUNC_NAME_ __FUNCTION__
#	define _LINE_NUM_ __LINE__
#endif

// TODO: rewrite this so all log handlers call into this function; MLOG_ERROR would pass false to condition, where as
// any other default would be "true". MLOG_ASSERT would pass strictly the condition specified by its caller, of course.
//void MLogError( bool condition, const char* filename, int32_t line, const char* funcname, ... );

#define MLOG_INFO( ... ) ( O_Log( ( _FUNC_NAME_ ), "INFO", __VA_ARGS__ ) )

#define MLOG_INFOB( ... ) ( O_LogBuffer( ( _FUNC_NAME_ ), "INFO", __VA_ARGS__ ) )

#define MLOG_ERROR( ... )									\
	do                                                      \
	{                                                       \
		O_Log( ( _FUNC_NAME_ ), "ERROR", __VA_ARGS__ );     \
		FlagExit();                                         \
	}                                                       \
	while( 0 )

#define MLOG_WARNING( ... ) ( O_Log( ( _FUNC_NAME_ ), "WARNING", __VA_ARGS__ ) )

#define MLOG_WARNING_SANS_FUNCNAME( title, ... ) ( O_Log( ( title ), "WARNING", __VA_ARGS__ )  )

#ifdef DEBUG
#	define MLOG_ASSERT( condition, ... )		\
		do                                      \
		{                                       \
			if ( !( condition ) )               \
			{                                   \
				MLOG_ERROR( __VA_ARGS__ );      \
			}                                   \
		}                                       \
		while( 0 )
#else
#	define MLOG_ASSERT( condition, ... ) ()
#endif

template < typename T >
INLINE bool File_GetBuf( std::vector< T >& outBuffer, const std::string& fpath )
{
	FILE* f = fopen( fpath.c_str(), "rb" );
	if ( !f )
	{
		return false;
	}

	fseek( f, 0, SEEK_END );
	size_t count = ftell( f ) / sizeof( T );
	fseek( f, 0, SEEK_SET );

	outBuffer.resize( count + 1, 0 );
	fread( &outBuffer[ 0 ], sizeof( T ), count, f );
	fclose( f );

	return true;
}

/*!
	Provides the file extension of a file, without the period.
	A return of true indicates we have an extension; we also allow for the index to be returned
	for the rare case that we want to do something specific in the same
	location. It's totally optional though
*/
static INLINE bool File_GetExt( std::string& outExt, size_t* outIndex, const std::string& filename  )
{
	// Second condition is to ensure we actually have a file extension we can use
	size_t index = filename.find_last_of( '.' );
	if ( index != std::string::npos && index != filename.size() - 1 )
	{
		outExt = filename.substr( index + 1 );
		if ( outIndex )
			*outIndex = index;

		return true;
	}

	return false;
}


bool File_GetPixels( const std::string& filepath,
	std::vector< uint8_t >& outBuffer, int32_t& outBpp, int32_t& outWidth, int32_t& outHeight );

/// Convert a buffer with an arbitrary bytes per pixel into an RGBA equivalent
/// buffer. Any  RGB channels NOT included in the source pixel format are zerod out,
/// while the alpha level is left untouched.
static INLINE void Pixels_To32Bit( uint8_t* destination,
								   const uint8_t* source,
								   uint8_t sourceBPP,
								   int32_t numPixels )
{
	for ( int32_t i = 0; i < numPixels; ++i )
	{
		for ( uint8_t k = 0; k < sourceBPP; ++k )
			destination[ i * 4 + k ] = source[ i * sourceBPP + k ];

		for ( uint8_t k = sourceBPP; k < 3; ++k )
			destination[ i * 4 + k ] = 0;

		/*
		destination[ i * 4 + 0 ] = source[ i * 3 + 0 ];
		destination[ i * 4 + 1 ] = source[ i * 3 + 1 ];
		destination[ i * 4 + 2 ] = source[ i * 3 + 2 ];
		*/
	}
}

/// Convert a buffer with an arbitrary bytes per pixel into an R equivalent
/// buffer. The channel param passed refers to the color channel for each pixel
/// in the source buffer we want to fetch.
static INLINE void Pixels_ToR( uint8_t* destination,
							   const uint8_t* source,
							   uint8_t sourceBPP,
							   uint8_t channel,
							   int32_t numPixels )
{
	for ( int32_t i = 0; i < numPixels; ++i )
	{
		destination[ i ] = source[ i * sourceBPP + channel ];
	}
}
