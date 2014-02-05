#include "trenderer.h"
#include "../input.h"

static BSPRenderer renderer;
static Input input;

static double currTime, prevTime;
static bool cursorVisible;

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
                case GLFW_KEY_F1:
                    cursorVisible = !cursorVisible;

                    if ( cursorVisible )
                    {
                        glfwSetInputMode( w, GLFW_CURSOR, GLFW_CURSOR_NORMAL );
                    }
                    else
                    {
                        glfwSetInputMode( w, GLFW_CURSOR, GLFW_CURSOR_DISABLED );
                    }

                    break;
                default:
                    input.EvalKeyPress( key );
                    break;
            }
            break;

        case GLFW_RELEASE:
            input.EvalKeyRelease( key );
            break;

        default:

            break;
    }

}

void HandleMousePosTestRenderer( GLFWwindow* w, double x, double y )
{
    input.EvalMouseMove( ( float ) x, ( float ) y );
}

void LoadTestRenderer( GLFWwindow* window )
{
    prevTime = glfwGetTime();

    glfwSetKeyCallback( window, HandleInputTestRenderer );
    glfwSetCursorPosCallback( window, HandleMousePosTestRenderer );
    glfwSetInputMode( window, GLFW_STICKY_KEYS, GL_FALSE );
   // glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );

    renderer.Prep();
    renderer.Load( "asset/quake/railgun_arena/map.bsp" );

    cursorVisible = true;
}

void DrawTestRenderer( void )
{
    RenderPass pass( input.LastPass() );

    currTime = glfwGetTime();

    renderer.Update( currTime - prevTime, pass );
    renderer.Draw( pass );

    input.UpdatePass( pass );

    prevTime = currTime;
}
