#include "log.h"
#include "q3m.h"
#include "gldebug.h"

FILE* drawLog = NULL;
FILE* bspDataLog = NULL;

int* meshVertexOffsets = NULL;

void myPrintf( const char* header, const char* fmt, ... )
{
    va_list arg;

    va_start( arg, fmt );
    fprintf( stdout, "\n[ %s ]: {\n\n", header );
    vfprintf( stdout, fmt, arg );
    fprintf( stdout, "\n\n}\n\n" );
    va_end( arg );
}

void myFPrintF( FILE* f, const char* header, const char* fmt, ... )
{
    va_list arg;

    va_start( arg, fmt );
    fprintf( f, "\n[ %s ]: {\n\n", header );
    vfprintf( f, fmt, arg );
    fprintf( f, "\n\n}\n\n" );
    va_end( arg );
}

void myDateTime( const char* format, char* outBuffer, int length )
{
    time_t timer;
    struct tm* info;

    time( &timer );

    info = localtime( &timer );

    strftime( outBuffer, length, format, info );
}

void exitOnGLError( const char* caller )
{
    GLenum error = glGetError();

    if ( GL_NO_ERROR != error )
    {
        const char* errorString = ( const char* ) gluErrorString( error );

        myPrintf( caller, "GL ERROR: %s", errorString );
        flagExit();
    }
}

void initLogBaseData( Quake3Map* map )
{
    meshVertexOffsets = ( int* )malloc( sizeof( int ) * map->numMeshVertexes );
    memset( meshVertexOffsets, 0, sizeof( int ) * map->numFaces );
}

void logDrawCall( int faceIndex, const glm::vec3& camPos, const BSPFace* const face, const Quake3Map* const map )
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

    myPrintf( "YOU HAS A DRAW",
              "%s",
              ss.str().c_str() );
}

void logBspData( BspDataType type, void* data, int length )
{
    if ( !bspDataLog )
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

    myFPrintF( bspDataLog, header.c_str(), ss.str().c_str() );
}

void initLog( void )
{
    drawLog = fopen( "log/drawLog.log", "w" );
    bspDataLog = fopen( "log/bspData.log", "w" );

    if ( !drawLog )
        ERROR( "could not open gDrawLog" );

    if ( !bspDataLog )
        ERROR( "could not open gBspDataLog" );

    glDebugInit();
}

void killLog( void )
{
    if ( drawLog )
        fclose( drawLog );

    if ( bspDataLog )
        fclose( bspDataLog );

    if ( meshVertexOffsets )
        free( meshVertexOffsets );

    glDebugKill();
}

