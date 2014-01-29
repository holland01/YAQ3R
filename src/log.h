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

class BSPFace;
class BSPMeshVertex;
class Quake3Map;

enum BSPLogType
{
    BSP_LOG_VERTEXES,
    BSP_LOG_MESH_VERTEXES
};

void LogDrawCall( int faceIndex, const glm::vec3& camPos, const BSPFace* const face, const Quake3Map* const map );
void LogBSPData( BSPLogType type, void* data, int length );

void MyPrintf( const char* header, const char* fmt, ... );
void MyFprintf( FILE* f, const char* header, const char* fmt, ... );
void MyDateTime( const char* format, char* outBuffer, int length );

void ExitOnGLError( const char* caller );

void InitLog( void );
void InitLogBSPData( Quake3Map* map );

void KillLog( void );

#define ERROR_INFO_STR "Call made from file %s, in function %s, on line %iu"
#define ERROR( args... )                                \
    do                                                      \
    {                                                       \
        puts("======== ERROR ========");                    \
        myPrintf( ( __func__ ), args );                   \
        puts("=======================");                    \
        flagExit();                                         \
    }                                                       \
    while( 0 )

#define WARNING( args... )                              \
    do                                                      \
    {                                                       \
        puts("======== WARNING ========");                  \
        myPrintf( ( __func__ ), args );                   \
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
