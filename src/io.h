#pragma once

#include "common.h"
#include "em_api.h"

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

// Returns true if a trailing slash is needed in the path, otherwise false.
// Either way, the default OS_PATH_SEPARATOR is thrown in outSlash,
// or an alternative separator if the string already contains it (i.e., we're on Windows and
// the path isn't using back slashes...)
bool NeedsTrailingSlash( const std::string& path, char& outSlash );

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
		while ( 0 )
#elif defined (DEBUG_RELEASE)					
#	define MLOG_ASSERT( condition, ... )	\
	do                                      \
	{                                       \
		if ( !( condition ) )               \
		{                                   \
			O_Log( _FUNC_NAME_, 			\
				"ASSERT", __VA_ARGS__ );    \
			FlagExit();						\
		}                                   \
	}                                       \
	while ( 0 )
#else
#	define MLOG_ASSERT( condition, ... )
#endif

enum fileCommand_t
{
	FILE_CONTINUE_TRAVERSAL = 1,
	FILE_STOP_TRAVERSAL = 0
};

// This is a bit dirty, I know, but it's a simple method for integrating
// support for web workers.
#ifdef EM_USE_WORKER_THREAD
void File_QueryAsync( const std::string& fpath, em_worker_callback_func callback,
	void* param );

using filedata_t = char*;
typedef void ( *fileSystemTraversalFn_t )( filedata_t data, int size, void* arg );

#else
using filedata_t = uint8_t*;
// A return value of true (1) means "keep iterating, unless we're at the end";
// false will terminate the iteration
typedef int ( *fileSystemTraversalFn_t )( const filedata_t data );
#endif

void File_IterateDirTree( std::string directory, fileSystemTraversalFn_t callback );

FILE* File_Open( const std::string& path, const std::string& mode = "rb" );

// This is still useful in Emscripten for synchronous file io.
template < typename T >
INLINE bool File_GetBuf( std::vector< T >& outBuffer, const std::string& fpath )
{
	FILE* f = File_Open( fpath );

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
static INLINE bool File_GetExt( std::string& outExt, size_t* outIndex,
	const std::string& filename )
{
	// Second condition is to ensure we actually have a file extension we can use
	size_t index = filename.find_last_of( '.' );
	if ( index != std::string::npos && index != filename.size() - 1 )
	{
		outExt = filename.substr( index + 1 );
		if ( outIndex )
		{
			*outIndex = index;
		}

		return true;
	}

	return false;
}

static INLINE std::string File_StripPath( const std::string& path )
{
	// We check for both path separators, considering that having two separators simultaneously
	// is totally possible anywhere.
	size_t slash = path.find_last_of( '/' );
	size_t criminal = path.find_last_of( '\\' );

	size_t index = std::string::npos;
	if ( slash != std::string::npos && criminal != std::string::npos )
	{
		// Cleanse this path of its terrible affliction due to the abysmal
		// hell that is the result of having too many standards
		std::string tmp( path );
		index = slash;
		for ( uint32_t i = 0; i < tmp.length(); ++i )
		{
			if ( tmp[ i ] == '\\' )
			{
				tmp[ i ] = '/';
			}
		}

		index = tmp.find_last_of( '/' );
	}

	// Maybe we didn't have both at the same time, so
	// find which one is used ( if at all )
	if ( index == std::string::npos )
	{
		if ( slash != std::string::npos )
		{
			index = slash;
		}
		else if ( criminal != std::string::npos )
		{
			index = criminal;
		}
		else
		{
			return path;
		}
	}

	// One final validation...
	if ( index == path.size() - 1 )
	{
		return path;
	}

	return path.substr( index + 1 );
}

static INLINE std::string File_StripExt( const std::string& name )
{
	size_t index = name.find_last_of( '.' );

	if ( index == std::string::npos )
	{
		return name;
	}

	return name.substr( 0, index );
}

bool File_GetPixels( const std::string& filepath,
	std::vector< uint8_t >& outBuffer,
	int32_t& outBpp, int32_t& outWidth, int32_t& outHeight );

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

class LogHandle
{
public:
	FILE* ptr = nullptr;
	bool enabled;

	LogHandle( const std::string& path, bool _enabled );
	~LogHandle( void );
};
