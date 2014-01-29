#include "log.h"
#include "q3m.h"
#include "gldebug.h"

FILE* globalDrawLog = NULL;
FILE* globalBspDataLog = NULL;

/*
===============================

MyPrintf

===============================
*/

void MyPrintf( const char* header, const char* fmt, ... )
{
    va_list arg;

    va_start( arg, fmt );
    fprintf( stdout, "\n[ %s ]: {\n\n", header );
    vfprintf( stdout, fmt, arg );
    fprintf( stdout, "\n\n}\n\n" );
    va_end( arg );
}

/*
===============================

MyFprintf

===============================
*/

void MyFprintf( FILE* f, const char* header, const char* fmt, ... )
{
    va_list arg;

    va_start( arg, fmt );
    fprintf( f, "\n[ %s ]: {\n\n", header );
    vfprintf( f, fmt, arg );
    fprintf( f, "\n\n}\n\n" );
    va_end( arg );
}

/*
===============================

MyDateTime

===============================
*/

void MyDateTime( const char* format, char* outBuffer, int length )
{
    time_t timer;
    struct tm* info;

    time( &timer );

    info = localtime( &timer );

    strftime( outBuffer, length, format, info );
}

/*
===============================

ExitOnGLError

    Perform a manual check for an OpenGL error
    and exit if that is the case. Useful in tracking down
    specific kinds of GL-related issues.

===============================
*/

void ExitOnGLError( const char* caller )
{
    GLenum error = glGetError();

    if ( GL_NO_ERROR != error )
    {
        const char* errorString = ( const char* ) gluErrorString( error );

        MyPrintf( caller, "GL ERROR: %s", errorString );
        flagExit();
    }
}

/*
===============================

LogDrawCall

===============================
*/

void LogDrawCall( int faceIndex, const glm::vec3& camPos, const BSPFace* const face, const Quake3Map* const map )
{
    std::stringstream ss;

    ss << "Camera Pos: { " << camPos.x << ", " << camPos.y << ", " << camPos.z << " }"
       << "face: " << faceIndex << ",\n"
       << "numMeshVertices: " << face->numMeshVertexes << ", \n"
       << "meshVertexOffset: " << face->meshVertexOffset << ", \n"
       << "( vertexes + meshVertexOffset )...( vertexes + meshVertexOffset + numMeshVertexes ): { \n\n";

    for ( int i = face->meshVertexOffset; i < face->meshVertexOffset + face->numMeshVertexes; ++i )
    {
        ss << "[ " << i << "]: { " << map->vertexes[ i ].position.x << ", "
                                   << map->vertexes[ i ].position.y << ", "
                                   << map->vertexes[ i ].position.z << " }\n";
    }

    ss << "\n } \n";

    MyPrintf( "YOU HAS A DRAW",
              "%s",
              ss.str().c_str() );
}

/*
===============================

LogBSPData

===============================
*/

void LogBSPData( BSPLogType type, void* data, int length )
{
    if ( !globalBspDataLog )
        return;

    std::stringstream ss;

    std::string header;

    switch( type )
    {
        case BSP_LOG_VERTEXES:
        {

            BSPVertex* vertexes = ( BSPVertex* ) data;

            header = "VERTEXES";

            ss << "TOTAL VERTEXES: " << length << "\n\n";

            for ( int i = 0; i < length; ++i )
            {
                ss << "Vertex [ " << i << " ]\n"
                   << "\t position:\n"
                   << "\t\t x: " << vertexes[ i ].position.x << "\n"
                   << "\t\t y: " << vertexes[ i ].position.y << "\n"
                   << "\t\t z: " << vertexes[ i ].position.z << "\n"
                   << "\t texcoords[ 0 ]:\n"
                   << "\t\t x: " << vertexes[ i ].texcoords[ 0 ].x << "\n"
                   << "\t\t y: " << vertexes[ i ].texcoords[ 0 ].y << "\n"
                   << "\t texcoords[ 1 ]:\n"
                   << "\t\t x: " << vertexes[ i ].texcoords[ 1 ].x << "\n"
                   << "\t\t y: " << vertexes[ i ].texcoords[ 1 ].y << "\n"
                   << "\t normal:\n"
                   << "\t\t x: " << vertexes[ i ].normal.x << "\n"
                   << "\t\t y: " << vertexes[ i ].normal.y << "\n"
                   << "\t\t z: " << vertexes[ i ].normal.z << "\n"
                   << "\t color byte:\n"
                   << "\t\t r: " << vertexes[ i ].color[ 0 ] << "\n"
                   << "\t\t g: " << vertexes[ i ].color[ 1 ] << "\n"
                   << "\t\t b: " << vertexes[ i ].color[ 2 ] << "\n"
                   << "\t\t a: " << vertexes[ i ].color[ 3 ] << "\n"
                   << "End Vertex\n\n";
            }
        }
            break;

        case BSP_LOG_MESH_VERTEXES:
        {

            BSPMeshVertex* meshVertexes = ( BSPMeshVertex* ) data;

            header = "MESH_VERTEXES";

            ss << "TOTAL MESH VERTEXES: " << length << "\n\n";

            for ( int i = 0; i < length; ++i )
            {
                ss << "Mesh Vertex [ " << i << " ]\n"
                   << "\t offset: " << meshVertexes[ i ].offset << "\n"
                   << "End Mesh Vertex\n\n";
            }
        }
            break;

    }

    MyFprintf( globalBspDataLog, header.c_str(), ss.str().c_str() );
}

void InitLog( void )
{
    globalDrawLog = fopen( "log/drawLog.log", "w" );
    globalBspDataLog = fopen( "log/bspData.log", "w" );

    if ( !globalDrawLog )
        ERROR( "could not open gDrawLog" );

    if ( !globalBspDataLog )
        ERROR( "could not open gBspDataLog" );

    glDebugInit();
}

void KillLog( void )
{
    if ( globalDrawLog )
        fclose( globalDrawLog );

    if ( globalBspDataLog )
        fclose( globalBspDataLog );

    glDebugKill();
}

