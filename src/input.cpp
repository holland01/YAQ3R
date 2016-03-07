#include "input.h"
#include "io.h"
#include <SDL2/SDL.h>

static const float MOUSE_SENSE = 0.1f;
static const float DEF_MOVE_STEP_SPEED = 10.0f;

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
    : viewData( view ),
      currRot( currRot ),
      lastMouse( 0.0f ),
      moveStep( DEF_MOVE_STEP_SPEED )
{
    for ( int i = 0; i < KEY_COUNT; ++i )
    {
        keysPressed[ i ] = KEY_NOT_PRESSED;
    }
}

InputCamera::InputCamera( float width, float height, const glm::mat4& view, const glm::mat4& projection )
{
	viewData.origin = glm::vec3( -view[ 3 ] );
	viewData.transform = view;
	viewData.clipTransform = projection;
	viewData.orientation = view;
	viewData.orientation[ 3 ] = glm::vec4( 0.0f, 0.0f, 0.0f, 1.0f );
	viewData.inverseOrient = glm::inverse( viewData.orientation );
	viewData.forward = Forward();
	viewData.up = Up();
	viewData.right = Right();
	viewData.width = width;
	viewData.height = height;
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
		case SDLK_w:
            keysPressed[ KEY_FORWARD ] = KEY_PRESSED;
            break;

		case SDLK_s:
            keysPressed[ KEY_BACKWARD ] = KEY_PRESSED;
            break;

		case SDLK_a:
            keysPressed[ KEY_LEFT ] = KEY_PRESSED;
            break;

		case SDLK_d:
            keysPressed[ KEY_RIGHT ] = KEY_PRESSED;
            break;

		case SDLK_LSHIFT:
            keysPressed[ KEY_DOWN ] = KEY_PRESSED;
            break;

		case SDLK_SPACE:
            keysPressed[ KEY_UP ] = KEY_PRESSED;
            break;

		case SDLK_e:
            keysPressed[ KEY_IN ] = KEY_PRESSED;
            break;

		case SDLK_q:
            keysPressed[ KEY_OUT ] = KEY_PRESSED;
            break;
    }
}

void InputCamera::EvalKeyRelease( int key )
{
	switch( key )
	{
		case SDLK_w:
			keysPressed[ KEY_FORWARD ] = KEY_NOT_PRESSED;
			break;

		case SDLK_s:
			keysPressed[ KEY_BACKWARD ] = KEY_NOT_PRESSED;
			break;

		case SDLK_a:
			keysPressed[ KEY_LEFT ] = KEY_NOT_PRESSED;
			break;

		case SDLK_d:
			keysPressed[ KEY_RIGHT ] = KEY_NOT_PRESSED;
			break;

		case SDLK_LSHIFT:
			keysPressed[ KEY_DOWN ] = KEY_NOT_PRESSED;
			break;

		case SDLK_SPACE:
			keysPressed[ KEY_UP ] = KEY_NOT_PRESSED;
			break;

		case SDLK_e:
			keysPressed[ KEY_IN ] = KEY_NOT_PRESSED;
			break;

		case SDLK_q:
			keysPressed[ KEY_OUT ] = KEY_NOT_PRESSED;
			break;
	}
}

void InputCamera::Update( void )
{
     //currRot.Normalize();

     lastRot = currRot;

    if ( keysPressed[ KEY_FORWARD ] ) Walk( moveStep );
    if ( keysPressed[ KEY_BACKWARD ] ) Walk( -moveStep );
    if ( keysPressed[ KEY_RIGHT ] ) Strafe( moveStep );
    if ( keysPressed[ KEY_LEFT ] ) Strafe( -moveStep );
    if ( keysPressed[ KEY_UP ] ) Raise( moveStep );
    if ( keysPressed[ KEY_DOWN ] ) Raise( -moveStep );
    if ( keysPressed[ KEY_IN ] ) currRot.roll += moveStep;
    if ( keysPressed[ KEY_OUT ] ) currRot.roll -= moveStep;

    //currRot.Normalize();

    viewData.orientation = glm::rotate( glm::mat4( 1.0f ), glm::radians( currRot.pitch ), glm::vec3( 1.0f, 0.0f, 0.0f ) );
    viewData.orientation = glm::rotate( viewData.orientation, glm::radians( currRot.yaw ), glm::vec3( 0.0f, 1.0f, 0.0f ) );
    viewData.orientation = glm::rotate( viewData.orientation, glm::radians( currRot.roll ), glm::vec3( 0.0f, 0.0f, 1.0f ) );

    viewData.inverseOrient = glm::inverse( viewData.orientation );

    //viewData.inverseOrient = quake * viewData.inverseOrient;

    viewData.transform = viewData.orientation * glm::translate( glm::mat4( 1.0f ), -viewData.origin );

    viewData.orientation = viewData.orientation;
}
