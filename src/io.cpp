#include "io.h"
#include <extern/stb_image.h>
#include <glm/gtx/string_cast.hpp>
#include <SDL2/SDL.h>

extern void FlagExit( void );

#ifdef _WIN32
#	define OS_PATH_SEPARATOR '\\'
#else
#	define OS_PATH_SEPARATOR '/'
#endif

#if defined( O_INTERVAL_LOGGING )
struct frameTick_t
{
	float accum = 0.0f;
	float interval = 5.0f;
	bool hitInterval = false;
};

static frameTick_t gFrameTick;

void O_IntervalLogUpdateFrameTick( float dt )
{
	gFrameTick.accum += glm::abs( dt );

	if ( gFrameTick.accum < gFrameTick.interval )
	{
		gFrameTick.hitInterval = false;
	}
	else
	{
		gFrameTick.accum = 0.0f;
		gFrameTick.hitInterval = true;
	}
}

void O_IntervalLogSetInterval( float interval )
{
	// Security? Maybe: if the interval < 0 then
	// hitInterval will always be true. Things slow down,
	// and potential doors open. There's far more that would need
	// to be done, but I'd consider this good practice.
	gFrameTick.interval = glm::abs( interval );
}
#else
void O_IntervalLogUpdateFrameTick( float dt )
{
	UNUSED( dt );
}

void O_IntervalLogSetInterval( float interval )
{
	UNUSED( interval );
}
#endif

void O_Log( const char* header, const char* priority, const char* fmt, ... )
{
#ifdef DEBUG
	va_list arg;

	va_start( arg, fmt );
	fprintf( stdout, "\n[ %s | %s ]: ", header, priority );
	vfprintf( stdout, fmt, arg );
	fputs( "\n", stdout );
	va_end( arg );
#elif defined( O_INTERVAL_LOGGING )
	if ( gFrameTick.hitInterval )
	{
		va_list arg;

		va_start( arg, fmt );
		fprintf( stdout, "\n[ %s | %s ]: ", header, priority );
		vfprintf( stdout, fmt, arg );
		fputs( "\n", stdout );
		va_end( arg );
	}
#else
	UNUSED( header );
	UNUSED( priority );
	UNUSED( fmt );
#endif
}

void O_LogBuffer( const char* header, const char* priority,
	const char* fmt, ... )
{
#ifdef DEBUG
	va_list arg;

	va_start( arg, fmt );
	fprintf( stdout, "\033[2J \033[H \n[ %s | %s ]: \n \033[H\n", header,
		priority );
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

		O_Log( _FUNC_NAME_, "DRAW ERROR",
			"%s -> [ %s ( %i ) ]: \'0x%x\' => %s\n",
			callerFunc, glFunc, line, error, errorString );
		FlagExit();
	}
}

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

bool File_GetPixels( const std::string& filepath,
	std::vector< uint8_t >& outBuffer, int32_t& outBpp, int32_t& outWidth,
	int32_t& outHeight )
{
	// Load image
	// Need to also flip the image, since stbi
	// loads pointer to upper left rather than
	// lower left (what OpenGL expects)
	uint8_t* imagePixels = stbi_load( filepath.c_str(), &outWidth,
		&outHeight, &outBpp,
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
