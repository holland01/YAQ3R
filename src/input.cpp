#include "input.h"
#include "renderer.h"
#include "log.h"

enum
{
    KEY_PRESSED = 1,
    KEY_NOT_PRESSED = 0,
    KEY_FORWARD = 0,
    KEY_BACKWARD = 1,
    KEY_LEFT = 2,
    KEY_RIGHT = 3,
    KEY_UP = 4,
    KEY_DOWN = 5
};

/*
=====================================================

Input::Input

=====================================================
*/

Input::Input( void )
{
    lastPass.position = glm::vec3( 0.0f );

    for ( int i = 0; i < KEY_COUNT; ++i )
    {
        keysPressed[ i ] = KEY_NOT_PRESSED;
    }
}

/*
=====================================================

Input::EvalMouseMove

=====================================================
*/

const float MOUSE_CAM_ROT_FACTOR = 0.1f;

void Input::EvalMouseMove( float x, float y )
{
    mouseX = glm::radians( x ) * MOUSE_CAM_ROT_FACTOR;
    mouseY = glm::radians( y ) * MOUSE_CAM_ROT_FACTOR;
}

/*
=====================================================

Input::EvalKeyPress

=====================================================
*/

void Input::EvalKeyPress( int key )
{
    switch( key )
    {
        case GLFW_KEY_W:
            keysPressed[ KEY_FORWARD ] = KEY_PRESSED;
            break;

        case GLFW_KEY_S:
            keysPressed[ KEY_BACKWARD ] = KEY_PRESSED;
            break;

        case GLFW_KEY_A:
            keysPressed[ KEY_LEFT ] = KEY_PRESSED;
            break;

        case GLFW_KEY_D:
            keysPressed[ KEY_RIGHT ] = KEY_PRESSED;
            break;

        case GLFW_KEY_LEFT_SHIFT:
            keysPressed[ KEY_DOWN ] = KEY_PRESSED;
            break;

        case GLFW_KEY_SPACE:
            keysPressed[ KEY_UP ] = KEY_PRESSED;
            break;
    }
}

/*
=====================================================

Input::EvalKeyRelease

=====================================================
*/

void Input::EvalKeyRelease( int key )
{
    switch( key )
    {
        case GLFW_KEY_W:
            keysPressed[ KEY_FORWARD ] = KEY_NOT_PRESSED;
            break;
        case GLFW_KEY_S:
            keysPressed[ KEY_BACKWARD ] = KEY_NOT_PRESSED;
            break;
        case GLFW_KEY_A:
            keysPressed[ KEY_LEFT ] = KEY_NOT_PRESSED;
            break;
        case GLFW_KEY_D:
            keysPressed[ KEY_RIGHT ] = KEY_NOT_PRESSED;
            break;
        case GLFW_KEY_LEFT_SHIFT:
            keysPressed[ KEY_DOWN ] = KEY_NOT_PRESSED;
            break;
        case GLFW_KEY_SPACE:
            keysPressed[ KEY_UP ] = KEY_NOT_PRESSED;
            break;
    }
}

/*
=====================================================

Input::UpdatePass

=====================================================
*/

const float VIEW_STEP_SPEED = 8.0f;

void Input::UpdatePass( RenderPass& pass )
{
    pass.rotation.x = mouseY;
    pass.rotation.y = mouseX;

    glm::vec4 moveVec( VIEW_STEP_SPEED, VIEW_STEP_SPEED, -VIEW_STEP_SPEED, 1.0f );

    const glm::mat4& orient = pass.Orientation();
    const glm::mat4& inverseOrient = glm::inverse( orient );

    // Each vector component is computed with one vector and then extracted individually.
    // This is because the inverseOrient transform can manipulate values in the vector
    // inadvertently for keys which haven't been pressed.

    moveVec = inverseOrient * moveVec;

    float& x = moveVec.x;
    float& y = moveVec.y;
    float& z = moveVec.z;

    x = keysPressed[ KEY_LEFT ] ? -x : keysPressed[ KEY_RIGHT ] ? x : 0.0f;
    y = keysPressed[ KEY_DOWN ] ? -y : keysPressed[ KEY_UP ] ? y : 0.0f;
    z = keysPressed[ KEY_BACKWARD ] ? -z : keysPressed[ KEY_FORWARD ] ? z : 0.0f;

    pass.position += glm::vec3( moveVec );

    pass.view = orient * glm::translate( glm::mat4( 1.0f ), -pass.position );

    lastPass = pass;

    MyPrintf( "Position", "x: %f\n y: %f\n z: %f", pass.position.x, pass.position.y, pass.position.z );
}
