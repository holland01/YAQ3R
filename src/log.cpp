#include "log.h"
#include "q3bsp.h"
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
        FlagExit();
    }
}

/*
===============================

LogDrawCall

===============================
*/

void LogDrawCall( int faceIndex, const glm::vec3& leafBoundsCenter, const glm::vec3& camPos, const glm::mat4& faceTransform, const bspFace_t* const face, const Q3BspMap* const map )
{
    std::stringstream ss;

    ss << "Camera Pos: { " << camPos.x << ", " << camPos.y << ", " << camPos.z << " }, \n"
       << "leafBoundsCenter: { " << leafBoundsCenter.x << ", " << leafBoundsCenter.y << ", " << leafBoundsCenter.z << " }, \n"
       << "Translation Vector: { " << faceTransform[ 3 ][ 0 ] << ", " << faceTransform[ 3 ][ 1 ] << ", " << faceTransform[ 3 ][ 2 ] << " }, \n"
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

void LogBSPData( int type, void* data, int length )
{
    ASSERT( globalBspDataLog != NULL, "globalBspDataLog is NULL!" );
    ASSERT( type >= 0x0 && type <= 0x10, "Type not within range [0, 16]! Value received: %i", type );

    std::stringstream ss;

    ss << "TOTAL: " << length << "\n\n";

    std::string header;

    switch( type )
    {
        case BSP_LUMP_VERTEXES:
        {

            bspVertex_t* vertexes = ( bspVertex_t* ) data;

            header = "VERTEXES";

            for ( int i = 0; i < length; ++i )
            {
                ss << "Vertex [ " << i << " ]\n"
                   << "\t position:\n"
                   << "\t\t x: " << vertexes[ i ].position.x << "\n"
                   << "\t\t y: " << vertexes[ i ].position.y << "\n"
                   << "\t\t z: " << vertexes[ i ].position.z << "\n"
                   << "\t texcoords[ 0 ]:\n"
                   << "\t\t x: " << vertexes[ i ].texCoord.x << "\n"
                   << "\t\t y: " << vertexes[ i ].texCoord.y << "\n"
                   << "\t texcoords[ 1 ]:\n"
                   << "\t\t x: " << vertexes[ i ].lightmapCoord.x << "\n"
                   << "\t\t y: " << vertexes[ i ].lightmapCoord.y << "\n"
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

        case BSP_LUMP_MESH_VERTEXES:
        {

            bspMeshVertex_t* meshVertexes = ( bspMeshVertex_t* ) data;

            header = "MESH_VERTEXES";

            for ( int i = 0; i < length; ++i )
            {
                ss << "Mesh Vertex [ " << i << " ]\n"
                   << "\t offset: " << meshVertexes[ i ].offset << "\n"
                   << "End Mesh Vertex\n\n";
            }
        }
            break;

        case BSP_LUMP_TEXTURES:
        {

            bspTexture_t* texbuf = ( bspTexture_t* ) data;

            header = "TEXTURE_FILES";

            for ( int i = 0; i < length; ++i )
            {
                ss  << "Begin Texture[ " << i << " ]" << "\n";
                ss  << "\tFilename: " << texbuf[ i ].filename << "\n"
                    << "\tContent Flags: " << texbuf[ i ].contentsFlags << "\n"
                    << "\tSurface Flags: " << texbuf[ i ].surfaceFlags << "\n";
                ss  << "End Texture\n\n";
            }
        }
            break;

        case BSP_LUMP_EFFECTS:
        {

            bspEffect_t* effectBuf = ( bspEffect_t* ) data;

            header = "EFFECT_SHADERS";

            for ( int i = 0; i < length; ++i )
            {
                ss  << "Begin Effect Shader[ " << i << " ]" << "\n";
                ss  << "\tFilename: " << effectBuf[ i ].filename << "\n"
                    << "\tBrush Index: " << effectBuf[ i ].brush << "\n"
                    << "\tUknown Integer Field: " << effectBuf[ i ].unknown << "\n";
                ss  << "End Effect Shader\n\n";
            }
        }
            break;

        case BSP_LUMP_ENTITIES:
        {
            header = "ENTITIES_LUMP";

            ss << ( char* )data;
        }
            break;



        default:
            ERROR( "Log functionality for data type index %i has not been implemented yet!", type );
            break;

    }

    MyFprintf( globalBspDataLog, header.c_str(), ss.str().c_str() );
}

void InitSysLog( void )
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

