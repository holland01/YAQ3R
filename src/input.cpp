#include "input.h"
#include "log.h"
#include "math_util.h"

static const float MOUSE_SENSE = 0.1f;
static const float DEF_MOVE_STEP_SPEED = 50.0f;

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

InputCamera::InputCamera( void )
    : InputCamera( viewParams_t(), EuAng() )
{
}

InputCamera::InputCamera( const viewParams_t& view, const EuAng& currRot )
	: lastMouse( 0.0f ),
	  moveStep( DEF_MOVE_STEP_SPEED ),
	  viewData( view ),
	  currRot( currRot )
{
	SetPerspective( 45.0f, 16.0f / 9.0f, 0.1f, 20000.0f );

    for ( int i = 0; i < KEY_COUNT; ++i )
    {
        keysPressed[ i ] = KEY_NOT_PRESSED;
    }
}

void InputCamera::EvalMouseMove( float x, float y )
{
    currRot.pitch += ( y - lastMouse.y ) * MOUSE_SENSE;
    currRot.yaw += ( x - lastMouse.x ) * MOUSE_SENSE;

    lastMouse.x = x;
    lastMouse.y = y;
}

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

void InputCamera::Update( void )
{
     currRot.Normalize();

     lastRot = currRot;

    if ( keysPressed[ KEY_FORWARD ] ) Walk( moveStep );
    if ( keysPressed[ KEY_BACKWARD ] ) Walk( -moveStep );
    if ( keysPressed[ KEY_RIGHT ] ) Strafe( moveStep );
    if ( keysPressed[ KEY_LEFT ] ) Strafe( -moveStep );
    if ( keysPressed[ KEY_UP ] ) Raise( moveStep );
    if ( keysPressed[ KEY_DOWN ] ) Raise( -moveStep );
    if ( keysPressed[ KEY_IN ] ) currRot.roll += moveStep;
    if ( keysPressed[ KEY_OUT ] ) currRot.roll -= moveStep;

    currRot.Normalize();

    viewData.orientation = glm::rotate( glm::mat4( 1.0f ), glm::radians( currRot.pitch ), glm::vec3( 1.0f, 0.0f, 0.0f ) );
    viewData.orientation = glm::rotate( viewData.orientation, glm::radians( currRot.yaw ), glm::vec3( 0.0f, 1.0f, 0.0f ) );
    viewData.orientation = glm::rotate( viewData.orientation, glm::radians( currRot.roll ), glm::vec3( 0.0f, 0.0f, 1.0f ) );

    viewData.inverseOrient = glm::inverse( viewData.orientation );

    viewData.transform = viewData.orientation * glm::translate( glm::mat4( 1.0f ), -viewData.origin );
}
