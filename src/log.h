#pragma once

#include "common.h"

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

enum BspDataType
{
    BSP_LOG_VERTEXES,
    BSP_LOG_MESH_VERTEXES
};

class BspFace;
class BspMeshVertex;
class Quake3Map;

void logDrawCall( int faceIndex, const glm::vec3& camPos, const BspFace* const face, const Quake3Map* const map );
void logBspData( BspDataType type, void* data, int length );

void myPrintf( const char* header, const char* fmt, ... );
void myFPrintF( FILE* f, const char* header, const char* fmt, ... );
void myDateTime( const char* format, char* outBuffer, int length );

void exitOnGLError( const char* caller );

void initLog( void );
void initLogBaseData( Quake3Map* map );

void killLog( void );




