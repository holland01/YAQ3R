#pragma once

#include "common.h"

// Writes to stdout are incredibly slow in browser, so we define a simple API
// for more explicit management ( and possibly other facilities ).
// Be sure to define O_INTERVAL_LOGGING, otherwise this
// becomes a no-op
void O_IntervalLogUpdateFrameTick( float dt );
void O_IntervalLogSetInterval( float interval );
bool O_IntervalLogHit( void );

void O_Log( const char* header, const char* priority, const char* fmt, ... );
void O_LogBuffer( const char* header, const char* priority,
	const char* fmt, ... );
void O_LogOnce( const char* header, const char* priority, const char* fmt, ... );
void O_LogF( FILE* f, const char* header, const char* fmt, ... );
void MyDateTime( const char* format, char* outBuffer, int length );

float GetTimeSeconds( void );

void ExitOnGLError( int line, const char* glFunc, const char* callerFunc );

// Returns true if a trailing slash is needed in the path, otherwise false.
// Either way, the default OS_PATH_SEPARATOR is thrown in outSlash,
// or an alternative separator if the string already contains it
// (i.e., we're on Windows and the path isn't using back slashes...)
bool NeedsTrailingSlash( const std::string& path, char& outSlash );

// TODO: rewrite this so all log handlers call into this function;
// MLOG_ERROR would pass false to condition, where as
// any other default would be "true". MLOG_ASSERT would pass
// strictly the condition specified by its caller, of course.
// void MLogError( bool condition, const char* filename, int32_t line,
// const char* funcname, ... );
#define MLOG_INFO_ONCE( ... ) ( O_LogOnce( ( _FUNC_NAME_ ), "INFO", __VA_ARGS__ ) )

#define MLOG_INFO( ... ) ( O_Log( ( _FUNC_NAME_ ), "INFO", __VA_ARGS__ ) )

#define MLOG_INFOB( ... ) ( O_LogBuffer( ( _FUNC_NAME_ ), "INFO", \
	__VA_ARGS__ ) )

#define MLOG_ERROR( ... )									\
	do                                                      \
	{                                                       \
		O_Log( ( _FUNC_NAME_ ), "ERROR", __VA_ARGS__ );     \
		FlagExit();                                         \
	}                                                       \
	while( 0 )

#define MLOG_WARNING( ... ) ( O_Log( ( _FUNC_NAME_ ), "WARNING", \
	__VA_ARGS__ ) )

#define MLOG_WARNING_SANS_FUNCNAME( title, ... ) ( O_Log( ( title ), \
	"WARNING", __VA_ARGS__ )  )

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

#define MLOG_OFFSET( type, member ) \
	do { \
		size_t count = ( size_t )( &( ( ( type* )0 )->member ) ); \
		printf( "OFFSET OF " #type "::" #member ": " F_SIZE_T "\n", count ); \
	} while ( 0 )


// Provides the file extension of a file, without the period.
// A return of true indicates we have an extension; we also allow
// for the index to be returned
// for the rare case that we want to do something specific in the same
// location. It's totally optional though
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
	// We check for both path separators,
	// considering that having two separators simultaneously
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
