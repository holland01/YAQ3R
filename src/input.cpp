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
	: InputCamera( 800.0f, 600.0f )
{
}

InputCamera::InputCamera( float width, float height )
	:   lastMouse( 0.0f ),
	  	moveStep( DEF_MOVE_STEP_SPEED )
{
	viewData.origin = glm::zero< glm::vec3 >();

	viewData.transform = glm::mat4( 1.0f );

	viewData.orientation = glm::mat4( 1.0f );
	viewData.inverseOrient = glm::mat4( 1.0f );

	viewData.forward = Forward();
	viewData.up = Up();
	viewData.right = Right();

	SetPerspective( 65.0f, width, height, 1.0f, 100.0f );

	moveStep = DEF_MOVE_STEP_SPEED;

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
		case SDLK_w:
			MLOG_INFO( "%s", "W Pressed" );
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

void InputCamera::Update( float moveStepScale )
{
	 //currRot.Normalize();
	moveStepScale = glm::abs( moveStepScale );

	if ( glm::isnan( moveStepScale ) || moveStepScale < 0.00001f )
	{
		moveStepScale = 1.0f;
	}

	lastRot = currRot;

	float step = moveStep * moveStepScale;

	if ( keysPressed[ KEY_FORWARD ] ) Walk( step );
	if ( keysPressed[ KEY_BACKWARD ] ) Walk( -step );
	if ( keysPressed[ KEY_RIGHT ] ) Strafe( step );
	if ( keysPressed[ KEY_LEFT ] ) Strafe( -step );
	if ( keysPressed[ KEY_UP ] ) Raise( step );
	if ( keysPressed[ KEY_DOWN ] ) Raise( -step );
	if ( keysPressed[ KEY_IN ] ) currRot.roll += step;
	if ( keysPressed[ KEY_OUT ] ) currRot.roll -= step;

	//currRot.Normalize();

	viewData.orientation = glm::rotate( glm::mat4( 1.0f ), glm::radians( currRot.pitch ), glm::vec3( 1.0f, 0.0f, 0.0f ) );
	viewData.orientation = glm::rotate( viewData.orientation, glm::radians( currRot.yaw ), glm::vec3( 0.0f, 1.0f, 0.0f ) );
	viewData.orientation = glm::rotate( viewData.orientation, glm::radians( currRot.roll ), glm::vec3( 0.0f, 0.0f, 1.0f ) );

	viewData.inverseOrient = glm::inverse( viewData.orientation );

	//viewData.inverseOrient = quake * viewData.inverseOrient;

	viewData.transform = viewData.orientation * glm::translate( glm::mat4( 1.0f ), -viewData.origin );

	viewData.orientation = viewData.orientation;
}
