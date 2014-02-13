#include "test_util.h"
#include "../input.h"

void OnKeyPress( GLFWwindow* window, int key, int scancode, int action, int mods, InputCamera& camera, bool& cursorVisible )
{
    switch ( action )
    {
        case GLFW_PRESS:
            switch ( key )
            {
                case GLFW_KEY_ESCAPE:
                    FlagExit();
                    break;

                case GLFW_KEY_F1:
                    cursorVisible = !cursorVisible;

                    if ( cursorVisible )
                        glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_NORMAL );
                    else
                        glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );

                    break;

                default:
                    camera.EvalKeyPress( key );
                    break;
            }
            break;

        case GLFW_RELEASE:
            camera.EvalKeyRelease( key );
            break;

        default:
            break;
    }
}


