#include "io.h"
#include "q3bsp.h"
#include <extern/stb_image.h>
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

void O_Log( const char* header, const char* priority, const char* fmt, ... )
{
#ifdef DEBUG
	va_list arg;

	va_start( arg, fmt );
	fprintf( stdout, "\n[ %s | %s ]: ", header, priority );
	vfprintf( stdout, fmt, arg );
	fputs( "\n", stdout );
	va_end( arg );
#else
	UNUSED( header );
	UNUSED( priority );
	UNUSED( fmt );
#endif
}

void O_LogBuffer( const char* header, const char* priority, const char* fmt, ... )
{
#ifdef DEBUG
	va_list arg;

	va_start( arg, fmt );
	fprintf( stdout, "\033[2J \033[H \n[ %s | %s ]: \n \033[H\n", header, priority );
	vfprintf( stdout, fmt, arg );
	fputs( "\n", stdout );
	va_end( arg );
#else
	UNUSED( header );
	UNUSED( priority );
	UNUSED( fmt );
#endif
}

void O_LogF( FILE* f, const char* header, const char* fmt, ... )
{
#ifdef DEBUG
	va_list arg;

	va_start( arg, fmt );
	fprintf( f, "\n[ %s ]: {\n\n", header );
	vfprintf( f, fmt, arg );
	fprintf( f, "\n\n}\n\n" );
	va_end( arg );
#else
	UNUSED( f );
	UNUSED( header );
	UNUSED( fmt );
#endif
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

		O_Log( _FUNC_NAME_, "DRAW ERROR", "%s -> [ %s ( %i ) ]: \'0x%x\' => %s\n", callerFunc, glFunc, line, error, errorString );
		FlagExit();
	}
}

void LogWriteAtlasTexture( std::stringstream& sstream,
						   const gTextureHandle_t& texHandle,
						   const shaderStage_t* stage )
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

		case BSP_LUMP_SHADERS:
		{

			bspShader_t* texbuf = ( bspShader_t* ) data;

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

		case BSP_LUMP_FOGS:
		{

			bspFog_t* effectBuf = ( bspFog_t* ) data;

			header = "EFFECT_SHADERS";

			for ( int i = 0; i < length; ++i )
			{
				ss  << "Begin Effect Shader[ " << i << " ]" << "\n";
				ss  << "\tFilename: " << effectBuf[ i ].name << "\n"
					<< "\tBrush Index: " << effectBuf[ i ].brush << "\n"
					<< "\tUknown Integer Field: " << effectBuf[ i ].visibleSide << "\n";
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

	O_LogF( gBspDataLog, header.c_str(), ss.str().c_str() );
}

void InitSysLog( void )
{
	gDrawLog = fopen( "log/drawLog.log", "r" );
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

#if defined( __linux__ ) && !defined( EMSCRIPTEN )
namespace {
	using ftwFunction_t = std::function< int( const char*, const struct stat*, int ) >;

	ftwFunction_t gLinuxCallback;

	extern "C" int invoke( const char* path, const struct stat* sb, int typeFlag )
	{
		// Linux's ftw() will halt traversal if result is non-zero, so we negate because
		// our convention uses 1 for continue, 0 for stop
		if ( gLinuxCallback )
			return !gLinuxCallback( path, sb, typeFlag );

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

void File_IterateDirTree( std::string directory, fileSystemTraversalFn_t callback )
{

#ifdef _WIN32
	WIN32_FIND_DATAA findFileData;
	HANDLE file;

	char slash;
	if ( NeedsTrailingSlash( directory, slash ) )
		directory.append(1, slash);

	file = FindFirstFileA( ( directory + "*" ).c_str(), &findFileData );
	int success = file != INVALID_HANDLE_VALUE;

	while ( success )
	{
		std::string path( directory + std::string( findFileData.cFileName ) );

		if ( !callback( ( const filedata_t )path.c_str() ) )
			break;

		success = FindNextFileA( file, &findFileData );
	}

#elif defined( __linux__ )

	gLinuxCallback = [ & ]( const char* fpath, const struct stat* sb, int typeFlag ) -> int
	{
		UNUSED( sb );
		UNUSED( typeFlag );

		std::string path( fpath );

		// 0 means "finished"; 1 tells us to keep searching
		return callback( ( const filedata_t )path.c_str() );
	};


	ftw( directory.c_str(), invoke, 3 );

#elif defined( EM_USE_WORKER_THREAD )
	// std string will only return a constant version of its c-string,
	// which won't be accepted by emscripten_call_worker. So, we just copy it...
	size_t bsize = sizeof( char ) * directory.length();
	char* buffer = ( char* ) alloca( bsize + 1 );
	memset( buffer, 0, bsize + 1 );
	memcpy( buffer, directory.data(), bsize );

	//emscripten_call_worker( gFileWebWorker.handle, "Traverse", buffer, bsize,
	//	callback, nullptr );

	gFileWebWorker.Await( callback, "Traverse", buffer, bsize, nullptr );

#else // Emscripten, without worker threads
	char errorMsg[ 256 ];
	memset( errorMsg, 0, sizeof( errorMsg ) );

	int ret = EM_ASM_ARGS(
		return Module.walkFileDirectory($0, $1, $2);
	, directory.c_str(), callback, errorMsg );

	if ( !ret )
	{
		printf( "[WORKER ERROR]: %s\n", errorMsg );
	}
#endif
}

FILE* File_Open( const std::string& path, const std::string& mode )
{
	FILE* f = fopen( path.c_str(), mode.c_str() );

	if ( !f )
	{
		MLOG_ERROR( "Could not open \'%s\'", path.c_str() );
	}

	return f;
}

bool File_GetPixels( const std::string& filepath,
	std::vector< uint8_t >& outBuffer, int32_t& outBpp, int32_t& outWidth, int32_t& outHeight )
{
	// Load image
	// Need to also flip the image, since stbi loads pointer to upper left rather than lower left (what OpenGL expects)
	uint8_t* imagePixels = stbi_load( filepath.c_str(), &outWidth, &outHeight, &outBpp,
		STBI_default );

	if ( !imagePixels )
	{
		return false;
	}

	outBuffer.resize( outWidth * outHeight * outBpp, 0 );
	memcpy( &outBuffer[ 0 ], imagePixels, outBuffer.size() );

	stbi_image_free( imagePixels );

	return true;
}

LogHandle::LogHandle( const std::string& path, bool _enabled )
	: ptr( File_Open( path, "wb" ) ),
	  enabled( _enabled )
{
}

LogHandle::~LogHandle( void )
{
	if ( ptr )
	{
		fclose( ptr );
	}
}
