#include "common.h"
#include "renderer.h"
#include "log.h"
#include "tests/texture.h"
#include "tests/trenderer.h"
#include "tests/jpeg.h"

namespace {

typedef void( *DrawFunc )( void );

struct ArgMap
{
    const char* arg;
    DrawFunc    func;
};

ArgMap tests[] =
{
    { "--texture", &TEX_DrawTest },
    { "--bsp", &BSPR_DrawTest },
    { "--jpeg", &JPEG_DrawTest }
};

GLFWwindow* appWindow = NULL;

bool running = false;

bool AppInit( void )
{
    if ( !glfwInit() )
        return false;

    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 2 );

    appWindow = glfwCreateWindow( 1366, 768, "BSP View", NULL, NULL );

    if ( !appWindow )
    {
        return false;
    }

    glfwMakeContextCurrent( appWindow );

    glewExperimental = true;
    GLenum response = glewInit();

    if ( response != GLEW_OK )
    {
        printf( "Could not initialize GLEW! %s", glewGetErrorString( response ) );
        return false;
    }

    // GLEW-dependent OpenGL error is pushed on init. This is a temporary hack to just pop
    // that error off the error stack.
    glGetError();

    glEnable( GL_DEPTH_TEST );
    glDepthFunc( GL_LEQUAL );

    InitLog();

    MyPrintf( "Init", "OpenGL Version: %i.%i",
              glfwGetWindowAttrib( appWindow, GLFW_CONTEXT_VERSION_MAJOR ),
              glfwGetWindowAttrib( appWindow, GLFW_CONTEXT_VERSION_MINOR ) );

    return true;
}

std::string ArgTableString( void )
{
    std::stringstream out;

    for ( int i = 0; i < SIGNED_LEN( tests ); ++i )
    {
        out << "\t[ " << i << " ]" << tests[ i ].arg << " \n";
    }

    return out.str();
}

}

// Is global
void FlagExit( void )
{
    running = false;
}

int main( int argc, char** argv )
{
    DrawFunc* drawFunction;

    if ( !AppInit() )
        return 1;

    running = true;

    if ( argc > 1 )
    {
        int i;

        for ( i = 0; i < SIGNED_LEN( tests ); ++i )
        {
            if ( strcmp( tests[ i ].arg, argv[ 1 ] ) == 0 )
            {
                drawFunction = &tests[ i ].func;
                break;
            }
        }

        // No valid argument specified
        if ( i == SIGNED_LEN( tests ) - 1 )
        {
            ERROR( "Error! '%s' is invalid.\n Valid Arguments:\n %s", argv[ 1 ], ArgTableString().c_str() );
        }
    }
    else
    {
        ERROR( "No test argument specified. Stop." );
    }

    glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );

    while( running && !glfwWindowShouldClose( appWindow ) )
    {
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        ( **drawFunction )();

        glfwSwapBuffers( appWindow );
        glfwPollEvents();
    }

    glfwTerminate();
    glfwDestroyWindow( appWindow );
    KillLog();

    return 0;
}


