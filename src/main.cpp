#include "common.h"
#include "renderer.h"
#include "log.h"
#include "tests/texture.h"
#include "tests/trenderer.h"

GLFWwindow* window = NULL;

bool running = false;

void flagExit( void )
{
    running = false;
    KillLog();
}

bool appInit( void )
{
    if ( !glfwInit() )
        return false;

    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 2 );

    window = glfwCreateWindow( 1366, 768, "BSP View", NULL, NULL );

    if ( !window )
    {
        glfwTerminate();
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
    // GLEW-dependent OpenGL error is thrown on init. This is a temporary hack to get rid of it,
    // so calls to "exitOnGLError" can be made.
    glGetError();

    glEnable( GL_DEPTH_TEST );
    glDepthFunc( GL_LEQUAL );

    InitLog();

    running = true;

    return true;
}


int main( int argc, char** argv )
{
    if ( !appInit() )
        return 1;

    loadTestRenderer( window );

    glClearColor( 0.3f, 0.0f, 0.0f, 1.0f );

    while( running && !glfwWindowShouldClose( window ) )
    {
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        drawTestRenderer();

        glfwSwapBuffers( window );
        glfwPollEvents();
    }

    glfwDestroyWindow( window );
    glfwTerminate();

    return 0;
}


