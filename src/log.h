#pragma once

#include "common.h"
#include "gldebug.h"

/*
=====================================================

Author: Holland Schutte
License: WTFPL

                    log.h

        Generic file for logging issues related
        to OpenGL, the BSP data, and the Renderer/Camera.

=====================================================
*/

struct bspFace_t;
struct bspMeshVertex_t;
class Q3BspMap;

void LogDrawCall( int faceIndex, const glm::vec3& leafBoundsCenter, const glm::vec3& camPos, const glm::mat4& faceTransform, const bspFace_t* const face, const Q3BspMap* const map );
void LogBSPData( int bspDataType, void* data, int length );

void MyPrintf( const char* header, const char* fmt, ... );
void MyFprintf( FILE* f, const char* header, const char* fmt, ... );
void MyDateTime( const char* format, char* outBuffer, int length );

void ExitOnGLError( const char* caller );

void InitSysLog( void );
void InitLogBSPData( Q3BspMap* map );

void KillSysLog( void );

#ifdef __GNUC__
#	define _FUNC_NAME_ __func__
#elif defined (_MSC_VER)
#	define _FUNC_NAME_ __FUNCTION__
#endif



#ifdef __GNUC__
#	define ERROR_INFO_STR "Call made from file %s, in function %s, on line %iu"
#	define ERROR( args... )                                \
		do                                                      \
		{                                                       \
			puts("======== ERROR ========");                    \
			MyPrintf( ( _FUNC_NAME_ ), args );                   \
			puts("=======================");                    \
			FlagExit();                                         \
		}                                                       \
		while( 0 )

#	define WARNING( args... )                              \
		do                                                      \
		{                                                       \
			puts("======== WARNING ========");                  \
			MyPrintf( ( _FUNC_NAME_ ), args );                   \
			puts("=======================");                    \
		}                                                       \
		while( 0 )

#	define ASSERT( condition, args... )    \
		do                                      \
		{                                       \
			if ( !( condition ) )               \
			{                                   \
				ERROR( args );                  \
			}                                   \
		}                                       \
		while( 0 )

#elif defined( _MSC_VER )
#	define ERROR_INFO_STR "Call made from file %s, in function %s, on line %iu"
#	define ERROR( ... )                                \
		do                                                      \
		{                                                       \
			puts("======== ERROR ========");                    \
			MyPrintf( ( _FUNC_NAME_ ), __VA_ARGS__ );                   \
			puts("=======================");                    \
			FlagExit();                                         \
		}                                                       \
		while( 0 )

#	define WARNING( ... )                              \
		do                                                      \
		{                                                       \
			puts("======== WARNING ========");                  \
			MyPrintf( ( _FUNC_NAME_ ), __VA_ARGS__ );                   \
			puts("=======================");                    \
		}                                                       \
		while( 0 )

#	define ASSERT( condition, ... )    \
		do                                      \
		{                                       \
			if ( !( condition ) )               \
			{                                   \
				ERROR( __VA_ARGS__ );           \
			}                                   \
		}                                       \
		while( 0 )
#endif // __GNUC__
