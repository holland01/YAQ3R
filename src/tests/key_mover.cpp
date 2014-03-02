#include "key_mover.h"

KeyMover::KeyMover( glm::vec3& positionRef, float moveStep )
    : position( positionRef ),
      step( moveStep )
{
    memset( keyStates, 0, sizeof( bool ) * NUM_KEYS );
}

void KeyMover::EvalKeyPress( int key )
{
    switch( key )
    {
        case GLFW_KEY_J:
            keyStates[ KEYOBJ_UP ] = true;
            break;

        case GLFW_KEY_K:
            keyStates[ KEYOBJ_DOWN ] = true;
            break;

        case GLFW_KEY_H:
            keyStates[ KEYOBJ_LEFT ] = true;
            break;

        case GLFW_KEY_L:
            keyStates[ KEYOBJ_RIGHT ] = true;
            break;

        case GLFW_KEY_I:
            keyStates[ KEYOBJ_FORWARD ] = true;
            break;

        case GLFW_KEY_O:
            keyStates[ KEYOBJ_BACKWARD ] = true;
            break;
    }
}

void KeyMover::EvalKeyRelease( int key )
{
    switch( key )
    {
        case GLFW_KEY_J:
            keyStates[ KEYOBJ_UP ] = false;
            break;

        case GLFW_KEY_K:
            keyStates[ KEYOBJ_DOWN ] = false;
            break;

        case GLFW_KEY_H:
            keyStates[ KEYOBJ_LEFT ] = false;
            break;

        case GLFW_KEY_L:
            keyStates[ KEYOBJ_RIGHT ] = false;
            break;

        case GLFW_KEY_I:
            keyStates[ KEYOBJ_FORWARD ] = false;
            break;

        case GLFW_KEY_O:
            keyStates[ KEYOBJ_BACKWARD ] = false;
            break;
    }
}

void KeyMover::Update( void )
{
    if ( keyStates[ KEYOBJ_UP ] )
    {
        position.y += step;
    }

    if ( keyStates[ KEYOBJ_DOWN ] )
    {
        position.y -= step;
    }

    if ( keyStates[ KEYOBJ_RIGHT ] )
    {
        position.x += step;
    }

    if ( keyStates[ KEYOBJ_LEFT ] )
    {
        position.x -= step;
    }

    if ( keyStates[ KEYOBJ_FORWARD ] )
    {
        position.z -= step;
    }

    if ( keyStates[ KEYOBJ_BACKWARD ] )
    {
        position.z += step;
    }
}
