#include "log.h"
#include "q3m.h"
#include "gldebug.h"

FILE* gDrawLog = NULL;
FILE* gBspDataLog = NULL;

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
    meshVertexOffsets = ( int* )malloc( sizeof( int ) * map->mTotalMeshVertexes );
    memset( meshVertexOffsets, 0, sizeof( int ) * map->mTotalFaces );
}

void logDrawCall( int faceIndex, const BspFace* const face, const BspMeshVertex* meshVertexBuffer )
{
    // Check to see if we've already logged this
    if ( meshVertexOffsets[ face->meshVertexOffset ] == 1 )
        return;

    meshVertexOffsets[ face->meshVertexOffset ] = 1;

    myFPrintF( gDrawLog,
              "Draw Call Data",
              "face: %i,\n face->numMeshVertices: %i,\n face->meshVertexOffset: %i,\n mMap->mMeshVertexes[ face->meshVertexOffset ].offset: %i",
              faceIndex, face->numMeshVertexes, face->meshVertexOffset, ( meshVertexBuffer + face->meshVertexOffset )->offset );
}

void logBspData( BspDataType type, void* data, int length )
{
    if ( !gBspDataLog )
        return;

    std::stringstream ss;

    std::string header;

    switch( type )
    {
        case BSP_LOG_VERTEXES:
        {

            BspVertex* vertexes = ( BspVertex* ) data;

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

            BspMeshVertex* meshVertexes = ( BspMeshVertex* ) data;

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

    myFPrintF( gBspDataLog, header.c_str(), ss.str().c_str() );
}

void initLog( void )
{
    gDrawLog = fopen( "log/drawLog.log", "w" );
    gBspDataLog = fopen( "log/bspData.log", "w" );

    if ( !gDrawLog )
        ERROR( "could not open gDrawLog" );

    if ( !gBspDataLog )
        ERROR( "could not open gBspDataLog" );

    glDebugInit();
}

void killLog( void )
{
    if ( gDrawLog )
        fclose( gDrawLog );

    if ( gBspDataLog )
        fclose( gBspDataLog );

    if ( meshVertexOffsets )
        free( meshVertexOffsets );

    glDebugKill();
}

