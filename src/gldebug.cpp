#include "gldebug.h"
#include "log.h"


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
    GLDEBUG_LOG_FILE
};

static char* dateTime           = NULL;
static int   fileEntryCount     = 0;
static int   stdoutEntryCount   = 0;
static FILE* glLog              = NULL;

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
    MyDateTime( "%Y/%m/%d %H:%M:%S", dateTime, DATE_TIME_STR_SIZE );
    dateTime = ( char* )malloc( sizeof( char ) * DATE_TIME_STR_SIZE );
    glLog    = fopen( "log/gl.log", "w" );

    if ( !glLog )
        ERROR( "Could not open gl.log" );

    glEnable( GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB );

    glDebugMessageCallbackARB( glDebugOutProc, NULL );

    //exitOnGLError( "glDebugInit" );
}

/*
===========================

glDebugKill

===========================
*/

void glDebugKill( void )
{
    fclose( glLog );

    if ( dateTime )
    {
        free( dateTime );
        dateTime = NULL;
    }

    glDisable( GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB );
    glDebugMessageCallbackARB( NULL, NULL );

}

/*
===========================

glDebugOutProc

    Process GL-sent callback info
    and output to user-specified file.

===========================
*/

GL_PROC void glDebugOutProc( GLenum source,
                    GLenum type,
                    GLuint id,
                    GLenum severity,
                    GLsizei length,
                    const GLchar* message,
                    void* userParam )
{
    // __dateTime - __fileEntryCount | __stdoutEntryCount

    const char* out_fmt = "[ %s - %i ] { \n\n"\
                            "\tSource: %s \n"\
                            "\tType: %s \n"\
                            "\tID: %d \n"\
                            "\tSeverity: %s \n"\
                            "\tMessage: %s \n\n } \n";


    char deb_source  [ 16 ];
    char deb_type    [ 20 ];
    char deb_sev     [ 5 ];

    switch( source )
    {
        case GL_DEBUG_SOURCE_API_ARB:               strcpy( deb_source, "OpenGL" ); break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB:     strcpy( deb_source, "Windows" );  break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER_ARB:   strcpy( deb_source, "Shader Compiler" ); break;
        case GL_DEBUG_SOURCE_THIRD_PARTY_ARB:       strcpy( deb_source, "Third Party" ); break;
        case GL_DEBUG_SOURCE_APPLICATION_ARB:       strcpy( deb_source, "Application" ); break;
        case GL_DEBUG_SOURCE_OTHER_ARB:             strcpy( deb_source, "Other" ); break;
    }

    switch( type )
    {
        case GL_DEBUG_TYPE_ERROR_ARB:               strcpy( deb_type, "Error" ); break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB: strcpy( deb_type, "Deprecated Behavior" ); break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB:  strcpy( deb_type, "Undefined Behavior" ); break;
        case GL_DEBUG_TYPE_PORTABILITY_ARB:         strcpy( deb_type, "Portability" ); break;
        case GL_DEBUG_TYPE_PERFORMANCE_ARB:         strcpy( deb_type, "Performance" ); break;
        case GL_DEBUG_TYPE_OTHER_ARB:               strcpy( deb_type, "Other" ); break;

    }

    switch( severity )
    {
        case GL_DEBUG_SEVERITY_HIGH_ARB:    strcpy( deb_sev, "High" ); break;
        case GL_DEBUG_SEVERITY_MEDIUM_ARB:  strcpy( deb_sev, "Medium" ); break;
        case GL_DEBUG_SEVERITY_LOW_ARB:     strcpy( deb_sev, "Low" ); break;
    }

    switch( values[ 0 ] )
    {
        case GLDEBUG_LOG_FILE:
        {
            MyFprintf( glLog,
                     "Log",
                     out_fmt,
                     dateTime,
                     fileEntryCount,
                     deb_source,
                     deb_type,
                     id,
                     deb_sev,
                     message );

            ++fileEntryCount;
        }
        break;

        case GLDEBUG_LOG_STDOUT:
        {
            MyPrintf(  "glio_debug_out",
                        out_fmt,
                        dateTime,
                        stdoutEntryCount,
                        deb_source,
                        deb_type,
                        id,
                        deb_sev,
                        message );

            ++stdoutEntryCount;
        }
        break;
    }
}



