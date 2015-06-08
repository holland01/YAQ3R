#include "gldebug.h"
#include "io.h"

enum GLDebugValue
{
    GLDEBUG_LOG_STDOUT = 0x1,
    GLDEBUG_LOG_FILE = 0x2
};

/*
===========================

Globals

===========================
*/

static GLDebugValue  values[] =
{
    GLDEBUG_LOG_STDOUT
};

static char* dateTime           = NULL;
static int   fileEntryCount     = 0;
static int   stdoutEntryCount   = 0;
static FILE* glLog              = NULL;

static std::string glErrorSourceFn = "NOT SPECIFIED";
static std::string glErrorCallFn = "NOT SPECIFIED";

/*
===========================

glDebugInit

Initialize functionality
used to print OpenGL info
to a file as soon as OpenGL
detects a warning or issue.

===========================
*/

const int DATE_TIME_STR_SIZE = 40;

void glDebugInit( void )
{
    dateTime = ( char* )malloc( sizeof( char ) * DATE_TIME_STR_SIZE );
	MyDateTime( "%Y/%m/%d %H:%M:%S", dateTime, DATE_TIME_STR_SIZE );

    glLog = fopen( "log/gl.log", "w" );

    if ( !glLog )
        MLOG_ERROR( "Could not open gl.log" );

#ifdef _DEBUG_USE_GL_ASYNC_CALLBACK
	glEnable( GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB );
	glDebugMessageCallbackARB( glDebugOutProc, NULL );
#endif
}

void glDebugKill( void )
{
    fclose( glLog );

    if ( dateTime )
    {
        free( dateTime );
        dateTime = NULL;
    }

#ifdef _DEBUG_USE_GL_ASYNC_CALLBACK
	glDisable( GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB );
    glDebugMessageCallbackARB( NULL, NULL );
#endif
}

void glDebugSetCallInfo( const std::string& glFn, const std::string& calleeFn )
{
	glErrorCallFn = glFn;
	glErrorSourceFn = calleeFn;
}

void GL_PROC glDebugOutProc( GLenum source,
                    GLenum type,
                    GLuint id,
                    GLenum severity,
                    GLsizei length,
                    const GLchar* message,
                    const void* userParam )
{
    const char* format = "[ %s: %s ] [ %s - %i ] { \n\n"\
                            "\tSource: %s \n"\
                            "\tType: %s \n"\
                            "\tID: %d \n"\
                            "\tSeverity: %s \n"\
                            "\tMessage: %s \n\n } \n";

    char msgSource[ 20 ];
    char msgType[ 20 ];
    char msgSeverity[ 20 ];

	memset( msgSource, 0, sizeof( source ) );
	memset( msgType, 0, sizeof( msgType ) );
	memset( msgSeverity, 0, sizeof( msgSeverity ) );

	bool doExit = type == GL_DEBUG_TYPE_ERROR_ARB || type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB;

    switch( source )
    {
        case GL_DEBUG_SOURCE_API_ARB:               strcpy( msgSource, "OpenGL" ); break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB:     strcpy( msgSource, "Windows" );  break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER_ARB:   strcpy( msgSource, "Shader Compiler" ); break;
        case GL_DEBUG_SOURCE_THIRD_PARTY_ARB:       strcpy( msgSource, "Third Party" ); break;
        case GL_DEBUG_SOURCE_APPLICATION_ARB:       strcpy( msgSource, "Application" ); break;
        case GL_DEBUG_SOURCE_OTHER_ARB:             strcpy( msgSource, "Other" ); break;
    }

    switch( type )
    {
        case GL_DEBUG_TYPE_ERROR_ARB:               strcpy( msgType, "Error" ); break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB: strcpy( msgType, "Deprecated Behavior" ); break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB:  strcpy( msgType, "Undefined Behavior" ); break;
        case GL_DEBUG_TYPE_PORTABILITY_ARB:         strcpy( msgType, "Portability" ); break;
        case GL_DEBUG_TYPE_PERFORMANCE_ARB:         strcpy( msgType, "Performance" ); return; break;
        case GL_DEBUG_TYPE_OTHER_ARB:               strcpy( msgType, "Other" ); break;

    }
	
    switch( severity )
    {
        case GL_DEBUG_SEVERITY_HIGH_ARB:    strcpy( msgSeverity, "High" ); break;
        case GL_DEBUG_SEVERITY_MEDIUM_ARB:  strcpy( msgSeverity, "Medium" ); break;
        case GL_DEBUG_SEVERITY_LOW_ARB:     strcpy( msgSeverity, "Low" ); break;
		default:							return;
    }

    switch( values[ 0 ] )
    {
        case GLDEBUG_LOG_FILE:
        {
            MyFprintf( glLog,
                     "Log",
                     format,
					 glErrorSourceFn.c_str(),
					 glErrorCallFn.c_str(),
                     dateTime,
                     fileEntryCount,
                     source,
                     msgType,
                     id,
                     msgSeverity,
                     message );

            ++fileEntryCount;
        }
        break;

        case GLDEBUG_LOG_STDOUT:
        {
            MyPrintf(  "glio_debug_out",
                        format,
						glErrorSourceFn.c_str(),
						glErrorCallFn.c_str(),
                        dateTime,
                        stdoutEntryCount,
                        source,
                        msgType,
                        id,
                        msgSeverity,
                        message );

            ++stdoutEntryCount;
        }
        break;
    }

	if ( doExit )
		FlagExit();
}



