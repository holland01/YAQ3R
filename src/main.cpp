#include "common.h"
#include "renderer.h"
#include "log.h"
#include "tests/texture.h"
#include "tests/trenderer.h"

GLFWwindow* window = NULL;

bool running = false;

void FlagExit( void )
{
    running = false;
    KillLog();
}

bool AppInit( void )
{
    if ( !glfwInit() )
        return false;

    window = glfwCreateWindow( 1366, 768, "BSP View", NULL, NULL );

    if ( !window )
    {
        FlagExit();
        return false;
    }

    glfwMakeContextCurrent( window );

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
              glfwGetWindowAttrib( window, GLFW_CONTEXT_VERSION_MAJOR ),
              glfwGetWindowAttrib( window, GLFW_CONTEXT_VERSION_MINOR ) );

    running = true;

    return true;
}

typedef  void( *DrawFunc )( void );

DrawFunc drawFunction;

int main( int argc, char** argv )
{
    if ( !AppInit() )
        return 1;

    if ( strcmp( argv[ 1 ], "--texture" ) == 0 )
    {
        LoadTestTexture( window );
        drawFunction = &DrawTestTexture;
    }
    else if ( strcmp( argv[ 1 ], "--bsp" ) == 0 )
    {
        LoadTestRenderer( window );
        drawFunction = &DrawTestRenderer;
    }
    else
    {
        ERROR( "No argument specified for test to run" );
    }

    glClearColor( 0.3f, 0.0f, 0.0f, 1.0f );

    while( running && !glfwWindowShouldClose( window ) )
    {
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        ( *drawFunction )();

        glfwSwapBuffers( window );
        glfwPollEvents();
    }

    glfwDestroyWindow( window );
    glfwTerminate();

    return 0;
}


