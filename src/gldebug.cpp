#include "gldebug.h"
#include "log.h"
/*
===========================

Global

===========================
*/

typedef enum sys_glio_value_e
{
    GLDEBUG_LOG_STDOUT = 0x1,

    GLDEBUG_LOG_FILE = 0x2
}
sys_glio_value_t;

static sys_glio_value_t  g_values[] =
{
    GLDEBUG_LOG_FILE
};

static char* g_date_time            = NULL;
static int   g_file_entry_count     = 0;
static int   g_stdout_entry_count   = 0;
static FILE* g_gl_log               = NULL;

/*
===========================

sys_glio_init

===========================
*/

#define DT_STR_SIZE 40

void glDebugInit( void )
{
    g_date_time = ( char* )malloc( sizeof( char ) * DT_STR_SIZE );
    myDateTime( "%Y/%m/%d %H:%M:%S", g_date_time, DT_STR_SIZE );
    g_gl_log    = fopen( "log/gl.log", "w" );

    if ( !g_gl_log )
        ERROR( "Could not open gl.log" );

    glEnable( GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB );

    glDebugMessageCallbackARB( glDebugOutProc, NULL );
}

/*
===========================

sys_glio_kill

===========================
*/

void glDebugKill( void )
{
    fclose( g_gl_log );

    if ( g_date_time )
    {
        free( g_date_time );
        g_date_time = NULL;
    }

    glDisable( GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB );
    glDebugMessageCallbackARB( NULL, NULL );

}

/*
===========================

sys_glio_debug_out

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

    switch( g_values[ 0 ] )
    {
        case GLDEBUG_LOG_FILE:
        {
            myFPrintF( g_gl_log,
                     "Log",
                     out_fmt,
                     g_date_time,
                     g_file_entry_count,
                     deb_source,
                     deb_type,
                     id,
                     deb_sev,
                     message );

            ++g_file_entry_count;
        }
        break;

        case GLDEBUG_LOG_STDOUT:
        {
            myPrintf(  "glio_debug_out",
                        out_fmt,
                        g_date_time,
                        g_stdout_entry_count,
                        deb_source,
                        deb_type,
                        id,
                        deb_sev,
                        message );

            ++g_stdout_entry_count;
        }
        break;
    }
}



