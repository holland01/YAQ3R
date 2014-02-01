#include "trenderer.h"

BSPRenderer renderer;

double currTime, prevTime;

void HandleInputTestRenderer( GLFWwindow* w, int key, int scancode, int action, int mods )
{

    switch ( action )
    {

        case GLFW_PRESS:
            switch( key )
            {
                case GLFW_KEY_ESCAPE:
                    FlagExit();
                    break;
                case GLFW_KEY_R:
                    renderer.camera.rotation = glm::vec3( 0.0f );
                    break;
                case GLFW_KEY_P:
                    renderer.camera.position = glm::vec3( 0.0f );
                    break;
                default:
                    renderer.camera.EvalKeyPress( key );
                    break;
            }
            break;

        case GLFW_RELEASE:
            renderer.camera.EvalKeyRelease( key );
            break;

        default:

            break;
    }

}

void HandleMousePosTestRenderer( GLFWwindow* w, double x, double y )
{
    renderer.camera.EvalMouseMove( ( float ) x, ( float ) y );
}

void LoadTestRenderer( GLFWwindow* window )
{
    prevTime = glfwGetTime();

    glfwSetKeyCallback( window, HandleInputTestRenderer );
    glfwSetCursorPosCallback( window, HandleMousePosTestRenderer );
    glfwSetInputMode( window, GLFW_STICKY_KEYS, GL_FALSE );
    glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );

    renderer.Prep();
    renderer.Load( "asset/quake/railgun_arena/map.bsp" );
    renderer.camera.position = glm::vec3( -216.0f, 288.0f, 50.0f );
}

void DrawTestRenderer( void )
{
    currTime = glfwGetTime();

    renderer.Update( currTime - prevTime );
    renderer.Draw();

    prevTime = currTime;
}
