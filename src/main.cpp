#include "common.h"
#include "renderer.h"

GLFWwindow* _window = NULL;

GLRenderer _renderer;

bool _running = false;

void flagExit( void )
{
    _running = false;
}

void handleInput( GLFWwindow* w, int key, int scancode, int action, int mods )
{
    if ( key == GLFW_KEY_ESCAPE )
    {
        flagExit();
    }
}

bool initGL( void )
{
    if ( !glfwInit() )
        return false;

    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );

    _window = glfwCreateWindow( 1366, 768, "BSP View", NULL, NULL );

    if ( !_window )
    {
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent( _window );

    glfwSetKeyCallback( _window, handleInput );

    glewExperimental = true;
    GLenum response = glewInit();

    if ( response != GLEW_OK )
    {
        printf( "Could not initialize GLEW! %s", glewGetErrorString( response ) );

        return false;
    }

    return true;
}


int main( int argc, char** argv )
{
    if ( !initGL() )
        return 1;

    _running = true;

    glClearColor( 1.0f, 0.0f, 0.0f, 1.0f );

    while( _running && !glfwWindowShouldClose( _window ) )
    {
        glClear( GL_COLOR_BUFFER_BIT );

        _renderer.draw();

        glfwSwapBuffers( _window );

        _renderer.update();

        glfwPollEvents();
    }

    glfwDestroyWindow( _window );
    glfwTerminate();

    return 0;
}


