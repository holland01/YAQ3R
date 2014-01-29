#include "trenderer.h"

BSPRenderer renderer;

void HandleInputTestRenderer( GLFWwindow* w, int key, int scancode, int action, int mods )
{

    switch ( key )
    {
        case GLFW_KEY_ESCAPE:
            flagExit();
            break;
        default:
            renderer.camera.EvalKeyPress( key );
            break;
    }

}

void HandleMousePosTestRenderer( GLFWwindow* w, double x, double y )
{
    //renderer.mCamera.evalMouseCoords( ( float ) x, ( float ) y );
}

void LoadTestRenderer( GLFWwindow* window )
{
    glfwSetKeyCallback( window, HandleInputTestRenderer );
    glfwSetCursorPosCallback( window, HandleMousePosTestRenderer );

    renderer.Prep();
    renderer.Load( "asset/quake/aty3dm1v2.bsp" );
}

void DrawTestRenderer( void )
{
    renderer.Update();
    renderer.Draw();
}
