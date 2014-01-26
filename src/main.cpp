#include "common.h"
#include "renderer.h"
#include "log.h"
#include "tests/texture.h"

GLFWwindow* window = NULL;

BSPRenderer renderer;

bool running = false;

void flagExit( void )
{
    running = false;
    killLog();
}

void handleInput( GLFWwindow* w, int key, int scancode, int action, int mods )
{
    if ( action == GLFW_PRESS )
    {
        switch ( key )
        {
            case GLFW_KEY_ESCAPE:
                flagExit();
                break;
            default:
                renderer.mCamera.evalKeyPress( key );
                break;
        }
    }
}

bool initGL( void )
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

    glfwSetKeyCallback( window, handleInputTestTexture );
    glfwSetCursorPosCallback( window, handleMousePosTestTexture );

    glEnable( GL_DEPTH_TEST );
    glDepthFunc( GL_LEQUAL );

    return true;
}


int main( int argc, char** argv )
{
    if ( !initGL() )
        return 1;

    initLog();

    running = true;

    loadTestTexture();

    //_renderer.allocBase();
    //_renderer.loadMap( "asset/quake/aty3dm1v2.bsp" );

    glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );

    while( running && !glfwWindowShouldClose( window ) )
    {
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        //_renderer.draw();
        //_renderer.update();

        drawTestTexture();

        glfwSwapBuffers( window );
        glfwPollEvents();
    }

    glfwDestroyWindow( window );
    glfwTerminate();

    return 0;
}


