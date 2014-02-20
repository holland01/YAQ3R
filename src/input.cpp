#include "input.h"
#include "log.h"
#include "math_util.h"

namespace
{
    const float MOUSE_SENSE = 0.1f;

    const float VIEW_STEP_SPEED = 15.0f;

    enum
    {
        KEY_PRESSED = 1,
        KEY_NOT_PRESSED = 0,
        KEY_FORWARD = 0,
        KEY_BACKWARD = 1,
        KEY_LEFT = 2,
        KEY_RIGHT = 3,
        KEY_UP = 4,
        KEY_DOWN = 5,
        KEY_IN = 6,
        KEY_OUT = 7
    };
}

/*
=====================================================

InputCamera::Input

=====================================================
*/

InputCamera::InputCamera( void )
    :
      lastMouse( 0.0f )
{
    viewData.projection = glm::perspective( 45.0f, 16.0f / 9.0f, 0.1f, 9000.0f );

    for ( int i = 0; i < KEY_COUNT; ++i )
    {
        keysPressed[ i ] = KEY_NOT_PRESSED;
    }
}

/*
=====================================================

InputCamera::EvalMouseMove

=====================================================
*/



void InputCamera::EvalMouseMove( float x, float y )
{
    currRot.pitch += ( y - lastMouse.y ) * MOUSE_SENSE;
    currRot.yaw += ( x - lastMouse.x ) * MOUSE_SENSE;

    lastMouse.x = x;
    lastMouse.y = y;
}

/*
=====================================================

InputCamera::EvalKeyPress

=====================================================
*/

void InputCamera::EvalKeyPress( int key )
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
        case GLFW_KEY_E:
            keysPressed[ KEY_IN ] = KEY_PRESSED;
            break;
        case GLFW_KEY_Q:
            keysPressed[ KEY_OUT ] = KEY_PRESSED;
            break;
    }
}

/*
=====================================================

InputCamera::EvalKeyRelease

=====================================================
*/

void InputCamera::EvalKeyRelease( int key )
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
        case GLFW_KEY_E:
            keysPressed[ KEY_IN ] = KEY_NOT_PRESSED;
            break;
        case GLFW_KEY_Q:
            keysPressed[ KEY_OUT ] = KEY_NOT_PRESSED;
            break;

    }
}

/*
=====================================================

InputCamera::Update

=====================================================
*/


void InputCamera::Update( void )
{
     currRot.Normalize();

     lastRot = currRot;

    if ( keysPressed[ KEY_FORWARD ] ) Walk( VIEW_STEP_SPEED );
    if ( keysPressed[ KEY_BACKWARD ] ) Walk( -VIEW_STEP_SPEED );
    if ( keysPressed[ KEY_RIGHT ] ) Strafe( VIEW_STEP_SPEED );
    if ( keysPressed[ KEY_LEFT ] ) Strafe( -VIEW_STEP_SPEED );
    if ( keysPressed[ KEY_UP ] ) Raise( VIEW_STEP_SPEED );
    if ( keysPressed[ KEY_DOWN ] ) Raise( -VIEW_STEP_SPEED );
    if ( keysPressed[ KEY_IN ] ) currRot.roll += VIEW_STEP_SPEED;
    if ( keysPressed[ KEY_OUT ] ) currRot.roll -= VIEW_STEP_SPEED;

    currRot.Normalize();

    viewData.orientation = glm::rotate( glm::mat4( 1.0f ), glm::radians( currRot.pitch ), glm::vec3( 1.0f, 0.0f, 0.0f ) );
    viewData.orientation = glm::rotate( viewData.orientation, glm::radians( currRot.yaw ), glm::vec3( 0.0f, 1.0f, 0.0f ) );
    viewData.orientation = glm::rotate( viewData.orientation, glm::radians( currRot.roll ), glm::vec3( 0.0f, 0.0f, 1.0f ) );

    viewData.inverseOrient = glm::inverse( viewData.orientation );

    viewData.transform = viewData.orientation * glm::translate( glm::mat4( 1.0f ), -viewData.origin );


    MyPrintf( "Camera Info", "pitch: %f, roll: %f, yaw: %f,\n position = { x: %f, y: %f, z: %f }", currRot.pitch, currRot.roll, currRot.yaw,
              viewData.origin.x, viewData.origin.y, viewData.origin.z );

}

/*
=====================================================

InputCamera::RotateX

=====================================================
*/
/*
void InputCamera::RotateX( float angRad )
{
    if ( glm::length( viewData.rotation ) == 0 )
    {
        viewData.rotation = MakeQuat( angRad,  );
        viewData.rotConj -=viewData.rotation;
    }
    else
    {
        view
    }
}

void    InputCamera::RotateY( float angRad );
void    InputCamera::RotateZ( float angRad );
*/
