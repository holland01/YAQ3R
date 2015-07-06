#pragma once

#include "common.h"
#include "gldebug.h"

struct bspFace_t;
struct bspMeshVertex_t;
class Q3BspMap;

void LogBSPData( int bspDataType, void* data, int length );

void MyPrintf( const char* header, const char* fmt, ... );
void MyFprintf( FILE* f, const char* header, const char* fmt, ... );
void MyDateTime( const char* format, char* outBuffer, int length );

void ExitOnGLError( int line, const char* glFunc, const char* callerFunc );

void InitSysLog( void );
void InitLogBSPData( Q3BspMap* map );

void KillSysLog( void );

#ifdef __GNUC__
#	define _FUNC_NAME_ __func__
#	define _LINE_NUM_ __LINE__
#elif defined (_MSC_VER)
#	define _FUNC_NAME_ __FUNCTION__
#	define _LINE_NUM_ __LINE__
#endif

#define ERROR_INFO_STR "Call made from file %s, in function %s, on line %iu"

#ifdef __GNUC__
#	define MLOG_ERROR( args... )                                \
		do                                                      \
		{                                                       \
			puts("======== ERROR ========");                    \
			MyPrintf( ( _FUNC_NAME_ ), args );                   \
			puts("=======================");                    \
			FlagExit();                                         \
		}                                                       \
		while( 0 )

#	define MLOG_WARNING( args... )                              \
		do                                                      \
		{                                                       \
			puts("======== WARNING ========");                  \
			MyPrintf( ( _FUNC_NAME_ ), args );                   \
			puts("=======================");                    \
		}                                                       \
		while( 0 )

#	define MLOG_ASSERT( condition, args... )    \
		do                                      \
		{                                       \
			if ( !( condition ) )               \
			{                                   \
				MLOG_ERROR( args );                  \
			}                                   \
		}                                       \
		while( 0 )

#elif defined( _MSC_VER )

#	define MLOG_ERROR( ... )                                \
		do                                                      \
		{                                                       \
			puts("======== ERROR ========");                    \
			MyPrintf( ( _FUNC_NAME_ ), __VA_ARGS__ );                   \
			puts("=======================");                    \
			FlagExit();                                         \
		}                                                       \
		while( 0 )

#	define MLOG_WARNING( ... )                              \
		do                                                      \
		{                                                       \
			puts("======== WARNING ========");                  \
			MyPrintf( ( _FUNC_NAME_ ), __VA_ARGS__ );                   \
			puts("=======================");                    \
		}                                                       \
		while( 0 )

#	define MLOG_WARNING_SANS_FUNCNAME( title, ... )                              \
		do                                                      \
		{                                                       \
			puts("======== WARNING ========");                  \
			MyPrintf( ( title ), __VA_ARGS__ );                 \
			puts("=======================");                    \
		}                                                       \
		while( 0 )

#	define MLOG_ASSERT( condition, ... )    \
		do                                      \
		{                                       \
			if ( !( condition ) )               \
			{                                   \
				MLOG_ERROR( __VA_ARGS__ );           \
			}                                   \
		}                                       \
		while( 0 )
#endif // __GNUC__

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

	outBuffer.resize( count, 0 );
	fread( &outBuffer[ 0 ], sizeof( T ), count, f );
	fclose( f );

	return true;
}

bool File_GetPixels( const std::string& filepath, 
	std::vector< uint8_t >& outBuffer, int32_t& outBpp, int32_t& outWidth, int32_t& outHeight );

INLINE void Pixels_24BitTo32Bit( uint8_t* destination, const uint8_t* source, int32_t length )
{
	for ( int32_t i = 0; i < length; ++i )
	{
		destination[ i * 4 + 0 ] = source[ i * 3 + 0 ];
		destination[ i * 4 + 1 ] = source[ i * 3 + 1 ];
		destination[ i * 4 + 2 ] = source[ i * 3 + 2 ];
	}
}