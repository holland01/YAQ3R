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

#define O_INTERVAL_LOGGING

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

bool O_IntervalLogHit( void )
{
	return gFrameTick.hitInterval;
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

bool O_IntervalLogHit( void ) { return false; }
#endif

#define SCAN_ARG_BUFF( buffer, bufsize, header, priority, fmt ) 						\
do { 																					\
	va_list arg;																		\
	va_start( arg, fmt );																\
	int size = snprintf( &buffer[ 0 ], bufsize, "\n[ %s | %s ]: ", header, priority );	\
	if( size < 0 ) 																		\
	{																					\
		printf( "(O_LogOnce) Attempt to print string of format \'%s\' failed\n", fmt );	\
		return;																			\
	}																					\
	vsnprintf( &buffer[ size ], bufsize - size, fmt, arg );								\
	va_end( arg );																		\
} while ( 0 )

#define O_LOG_OUT_BUFFER_LENGTH 4096

void O_Log( const char* header, const char* priority, const char* fmt, ... )
{
#if defined( DEBUG )
	if ( true )
#elif defined( O_INTERVAL_LOGGING )
	if ( gFrameTick.hitInterval )
#else	// release build, so only print error messages
	if ( strcmp( priority, "ERROR" ) == 0 )
#endif
	{ 
		char buffer[ O_LOG_OUT_BUFFER_LENGTH ];
		memset( buffer, 0, sizeof( buffer ) );

		SCAN_ARG_BUFF( buffer, O_LOG_OUT_BUFFER_LENGTH, header, priority, fmt );

		printf( "%s\n", &buffer[ 0 ] );
	}
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

static std::vector< std::string > gLogOnceEntries;

void O_LogOnce( const char* header, const char* priority, const char* fmt, ... )
{
	char buffer[ O_LOG_OUT_BUFFER_LENGTH ];
	memset( buffer, 0, sizeof( buffer ) );

	SCAN_ARG_BUFF( buffer, O_LOG_OUT_BUFFER_LENGTH, header, priority, fmt );

	std::string asString( &buffer[ 0 ], strlen( &buffer[ 0 ] ) );

	for ( const std::string& str: gLogOnceEntries )
	{
		if ( asString == str )
			return;
	}

	gLogOnceEntries.push_back( asString );
	
	printf( "(O_LogOnce) \n%s\n", &buffer[ 0 ] );
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
