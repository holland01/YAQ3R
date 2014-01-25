#include "common.h"
#include "renderer.h"
#include "log.h"

GLFWwindow* _window = NULL;

GLRenderer _renderer;

const float STEP_SPEED = 2.0f;

bool _running = false;

void flagExit( void )
{
    _running = false;
    killLog();
}

void handleInput( GLFWwindow* w, int key, int scancode, int action, int mods )
{
    switch ( key )
    {
        case GLFW_KEY_ESCAPE:
            flagExit();
            break;
        case GLFW_KEY_W:
            _renderer.mCamera.walk( STEP_SPEED );
            break;
        case GLFW_KEY_S:
            _renderer.mCamera.walk( -STEP_SPEED );
            break;
        case GLFW_KEY_A:
            _renderer.mCamera.strafe( -STEP_SPEED );
            break;
        case GLFW_KEY_D:
            _renderer.mCamera.strafe( STEP_SPEED );
            break;
        case GLFW_KEY_SPACE:
            _renderer.mCamera.raise( STEP_SPEED );
            break;
        case GLFW_KEY_C:
            _renderer.mCamera.raise( -STEP_SPEED );
            break;
    }
}

bool initGL( void )
{
    if ( !glfwInit() )
        return false;

    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 2 );

    _window = glfwCreateWindow( 1366, 768, "BSP View", NULL, NULL );

    if ( !_window )
    {
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent( _window );

    glewExperimental = true;
    GLenum response = glewInit();

    if ( response != GLEW_OK )
    {
        printf( "Could not initialize GLEW! %s", glewGetErrorString( response ) );
        return false;
    }

    glfwSetKeyCallback( _window, handleInput );

    return true;
}


int main( int argc, char** argv )
{
    if ( !initGL() )
        return 1;

    initLog();

    _running = true;

    _renderer.allocBase();
    _renderer.loadMap( "asset/quake/aty3dm1v2.bsp" );

    glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );

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


