#pragma once

#include "common.h"

/*
=====================================================

Author: Holland Schutte
License: WTFPL

                    log.h

        Generic file for logging issues related
        to OpenGL, the BSP data, and the Renderer/Camera.

=====================================================
*/

class bspFace_t;
class bspMeshVertex_t;
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

#define ERROR_INFO_STR "Call made from file %s, in function %s, on line %iu"
#define ERROR( args... )                                \
    do                                                      \
    {                                                       \
        puts("======== ERROR ========");                    \
        MyPrintf( ( __func__ ), args );                   \
        puts("=======================");                    \
        FlagExit();                                         \
    }                                                       \
    while( 0 )

#define WARNING( args... )                              \
    do                                                      \
    {                                                       \
        puts("======== WARNING ========");                  \
        MyPrintf( ( __func__ ), args );                   \
        puts("=======================");                    \
    }                                                       \
    while( 0 )

#define ASSERT( condition, args... )    \
    do                                      \
    {                                       \
        if ( !( condition ) )               \
        {                                   \
            ERROR( args );                  \
        }                                   \
    }                                       \
    while( 0 )
