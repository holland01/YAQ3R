#include "io.h"
#include "q3bsp.h"
#include "gldebug.h"
#include "extern/stb_image.h"
#include "renderer.h"
#include "effect_shader.h"
#include <glm/gtx/string_cast.hpp>
#include <SDL2/SDL.h>

#ifdef _WIN32
#	define OS_PATH_SEPARATOR '\\'
#else
#	define OS_PATH_SEPARATOR '/'
#endif


FILE* gDrawLog = NULL;
FILE* gBspDataLog = NULL;

void MyPrintf( const char* header, const char* fmt, ... )
{
    va_list arg;

    va_start( arg, fmt );
    fprintf( stdout, "\n[ %s ]: {\n\n", header );
    vfprintf( stdout, fmt, arg );
    fprintf( stdout, "\n\n}\n\n" );
    va_end( arg );
}

void MyFprintf( FILE* f, const char* header, const char* fmt, ... )
{
    va_list arg;

    va_start( arg, fmt );
    fprintf( f, "\n[ %s ]: {\n\n", header );
    vfprintf( f, fmt, arg );
    fprintf( f, "\n\n}\n\n" );
    va_end( arg );
}

void MyDateTime( const char* format, char* outBuffer, int length )
{
    time_t timer;
    struct tm* info;

    time( &timer );

    info = localtime( &timer );

    strftime( outBuffer, length, format, info );
}

static const float TO_SECONDS = 1.0f / 1000.0f;

float GetTimeSeconds( void )
{
	return TO_SECONDS * ( float )SDL_GetTicks();
}

// Shamelessly stolen from:
// https://code.google.com/p/glues/source/browse/trunk/glues/source/glues_error.c

namespace {
	struct tokenString_t
	{
	   GLenum token;
	   const char* message;
	};
}


void ExitOnGLError( int line, const char* glFunc, const char* callerFunc )
{
    GLenum error = glGetError();

    if ( GL_NO_ERROR != error )
    {
		// No use in statically allocating it for this use case...
		const tokenString_t errors[]=
		{
		   /* GL */
		   {GL_NO_ERROR, "no error"},
		   {GL_INVALID_ENUM, "invalid enumerant"},
		   {GL_INVALID_VALUE, "invalid value"},
		   {GL_INVALID_OPERATION, "invalid operation"},
		   {GL_OUT_OF_MEMORY, "out of memory"},
		   { ~(0u), NULL } /* end of list indicator */
		};

		const char* errorString = "unlisted error message.";

		for ( int i = 0; errors[i].message; ++i )
		{
			if ( error == errors[i].token )
			{
				errorString = errors[i].message;
				break;
			}
		}

        MyPrintf( "GL ERROR", "%s -> [ %s ( %i ) ]: \'0x%x\' => %s", callerFunc, glFunc, line, error, errorString );
        FlagExit();
    }
}   

void LogWriteAtlasTexture( std::stringstream& sstream,
                           const drawSurface_t& surf,
                           const gTextureHandle_t& texHandle,
                           const shaderStage_t* stage,
                           const mapData_t& data )
{
    if ( !stage || stage->textureIndex < 0 )
        return;

    const gTextureImage_t& img = GTextureImage( texHandle, stage->textureIndex );

    uint32_t i = 0;
    for ( char c: stage->texturePath )
    {
        i++;

        if ( c == 0 )
            break;
    }

    std::string texPath( stage->texturePath.begin(), stage->texturePath.begin() + i );

    sstream << "SURFACE INFO ENTRY BEGIN \n"
            << "=================================================================\n"
            << "[ MATERIAL IMAGE SLOT: " << texPath << " ] {\n"
            << "\t[ begin ] " << glm::to_string( img.stOffsetStart ) << "\n"
            << "\t[ end   ] " << glm::to_string( img.stOffsetEnd ) << "\n"
            << "\t[ dims ] " << glm::to_string( img.dims ) << "\n"
            << "}\n\n";

    LogWriteIndexBuffers( sstream, surf, texHandle, img, "Index Buffers for Surface Using " + texPath, data );

}

void LogWriteIndexBuffers( std::stringstream& stream,
                           const drawSurface_t& surf,
                           const gTextureHandle_t& texHandle,
                           const gTextureImage_t& texParams,
                           const std::string& title,
                           const mapData_t& data )
{
    const glm::vec2& invRowPitch = GTextureInverseRowPitch( texHandle );

    auto transform = [ &texParams, &invRowPitch ]( const glm::vec2& coords ) -> glm::vec2
    {
        return coords * invRowPitch * texParams.imageScaleRatio + texParams.stOffsetStart;
    };

    auto clamp = [ &transform, &texParams ]( const glm::vec2& coords, float x ) -> glm::vec2
    {
        return glm::clamp( transform( coords ), texParams.stOffsetStart, transform( glm::vec2( x ) ) );
    };

    stream << "[ " << title << " ] { \n";

    for ( uint32_t i = 0; i < surf.indexBuffers.size(); ++i )
    {
        stream << "\t[ Index Buffer " << i << " ] {\n";

        for ( int32_t j = 0; j < surf.indexBufferSizes[ i ]; ++j )
        {
            bspVertex_t* v = data.vertexes + surf.indexBuffers[ i ][ j ];

            stream  << "\t\t[ " << j << " ] {\n"
                    << "\t\t\t[ position ] " << glm::to_string( v->position ) << "\n"
                    << "\t\t\t[ normal ] " << glm::to_string( v->normal ) << "\n"
                    << "\t\t\t[ texcoords: image ] " << glm::to_string( v->texCoords[ 0 ] ) << "\n"
                    << "\t\t\t[ texcoords: clamp( image 1 ) ] " << glm::to_string( clamp( v->texCoords[ 0 ], 1.0f ) ) << "\n"
                    << "\t\t\t[ texcoords: clamp( image 0.99 ) ] " << glm::to_string( clamp( v->texCoords[ 1 ], 0.99f ) ) << "\n"
					// Not available for version of GLM used on Windows; should probably upgrade...
                    //<< "\t\t\t[ color ] " << glm::to_string( v->color ) << "\n"
                    << "\t\t}\n\n";

        }

        stream << "\t}\n\n";
    }

    stream << " } \n";
}

void LogBSPData( int type, void* data, int length )
{
    MLOG_ASSERT( gBspDataLog != NULL, "globalBspDataLog is NULL!" );
    MLOG_ASSERT( type >= 0x0 && type <= 0x10, "Type not within range [0, 16]! Value received: %i", type );

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
                   << "\t\t x: " << vertexes[ i ].texCoords[ 0 ].x << "\n"
                   << "\t\t y: " << vertexes[ i ].texCoords[ 0 ].y << "\n"
                   << "\t texcoords[ 1 ]:\n"
                   << "\t\t x: " << vertexes[ i ].texCoords[ 1 ].x << "\n"
                   << "\t\t y: " << vertexes[ i ].texCoords[ 1 ].y << "\n"
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
                ss  << "\tFilename: " << texbuf[ i ].name << "\n"
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
                ss  << "\tFilename: " << effectBuf[ i ].name << "\n"
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
            MLOG_WARNING( "Log functionality for data type index %i has not been implemented yet!", type );
            break;

    }

    MyFprintf( gBspDataLog, header.c_str(), ss.str().c_str() );
}

void InitSysLog( void )
{
    gDrawLog = fopen( "log/drawLog.log", "w" );
    gBspDataLog = fopen( "log/bspData.log", "w" );

    if ( !gDrawLog )
    {
        MLOG_ERROR( "could not open gDrawLog" );
        return;
    }

    if ( !gBspDataLog )
    {
        MLOG_ERROR( "could not open gBspDataLog" );
        return;
    }
}

void KillSysLog( void )
{
    if ( gDrawLog )
        fclose( gDrawLog );

    if ( gBspDataLog )
        fclose( gBspDataLog );
}

#ifdef __linux__
namespace {
    using ftwFunction_t = std::function< int( const char*, const struct stat*, int ) >;

    ftwFunction_t gLinuxCallback;

    extern "C" int invoke( const char* path, const struct stat* sb, int typeFlag )
    {
        //if ( gLinuxCallback )
            return gLinuxCallback( path, sb, typeFlag );

        return 1;
    }
}
#endif // __linux__

bool NeedsTrailingSlash( const std::string& path, char& outSlash )
{
	size_t location = path.find_last_of(OS_PATH_SEPARATOR);

	if (location == std::string::npos)
	{
#ifdef _WIN32
		location = path.find_last_of('/');
		if (location != std::string::npos)
		{
			outSlash = path[location];
			return location != path.length() - 1;
		}
#endif
		outSlash = OS_PATH_SEPARATOR;
		return false;
	}

	outSlash = path[location];
	return location != path.length() - 1;
}

static INLINE bool QueryCaller( const std::string& path, fileSystemTraversalFn_t callback )
{
	fileStat_t fs;
	fs.filepath = path;
	return callback( fs );
}

void File_IterateDirTree( std::string directory, fileSystemTraversalFn_t callback )
{
	
#ifdef _WIN32
    // Find shader files
    WIN32_FIND_DATAA findFileData;
    HANDLE file;

	char slash;
	if ( NeedsTrailingSlash( directory, slash ) )
		directory.append(1, slash);

    file = FindFirstFileA( ( directory + "*" ).c_str(), &findFileData );
    int success = file != INVALID_HANDLE_VALUE;

    while ( success )
    {
		if ( !QueryCaller( directory + std::string( findFileData.cFileName ), callback ) )
			break;
        
		success = FindNextFileA( file, &findFileData );
    }
#elif defined( __linux__ )

    gLinuxCallback = [ & ]( const char* fpath, const struct stat* sb, int typeFlag ) -> int
    {
        UNUSED( sb );
        UNUSED( typeFlag );

        // Finished?
        if ( !QueryCaller( std::string( fpath ), callback ) )
            return 1;

        // Nope, keep going
        return 0;
    };


    ftw( directory.c_str(), invoke, 3 );
#else
	UNUSED( directory );
	UNUSED( callback );
	UNUSED( QueryCaller );

	MLOG_ERROR( "This needs Emscripten support..." );
#endif
}

bool File_GetPixels( const std::string& filepath, 
	std::vector< uint8_t >& outBuffer, int32_t& outBpp, int32_t& outWidth, int32_t& outHeight )
{
	// Load image
	// Need to also flip the image, since stbi loads pointer to upper left rather than lower left (what OpenGL expects)
	byte* imagePixels = stbi_load( filepath.c_str(), &outWidth, &outHeight, &outBpp, STBI_default );

	if ( !imagePixels )
	{
		MLOG_WARNING( "No file found for \'%s\'", filepath.c_str() );
		return false;
	}
	
    outBuffer.resize( outWidth * outHeight * outBpp, 0 );
	memcpy( &outBuffer[ 0 ], imagePixels, outBuffer.size() ); 

	stbi_image_free( imagePixels );

	return true;
}
