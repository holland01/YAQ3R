#include "trenderer.h"

BSPRenderer renderer;

void handleInputTestRenderer( GLFWwindow* w, int key, int scancode, int action, int mods )
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

void handleMousePosTestRenderer( GLFWwindow* w, double x, double y )
{
    //renderer.mCamera.evalMouseCoords( ( float ) x, ( float ) y );
}

void loadTestRenderer( GLFWwindow* window )
{
    glfwSetKeyCallback( window, handleInputTestRenderer );
    glfwSetCursorPosCallback( window, handleMousePosTestRenderer );

    renderer.allocBase();
    renderer.loadMap( "asset/quake/aty3dm1v2.bsp" );
}

void drawTestRenderer( void )
{
    renderer.update();
    renderer.draw();
}
