#pragma once

#include "common.h"
#include "eu_ang.h"

static const int KEY_COUNT = 8;

struct ViewParams
{
    glm::vec3   forward;
    glm::vec3   up;
    glm::vec3   right;

    glm::vec3   origin;

    glm::mat4   transform;

    glm::mat4   orientation;
    glm::mat4   inverseOrient;

    glm::mat4   projection;
};

class InputCamera
{
    ViewParams      viewData;

    EuAng           currRot, lastRot;

    glm::vec3       lastMouse;

    byte            keysPressed[ KEY_COUNT ];

public:

    InputCamera( void );

    void    EvalKeyPress( int key );
    void    EvalKeyRelease( int key );
    void    EvalMouseMove( float x, float y );

    void    Update( void );

    void    Walk( float amount );
    void    Strafe( float amount );
    void    Raise( float amount );

    void    RotateX( float angRad );
    void    RotateY( float angRad );
    void    RotateZ( float angRad );

    void    SetForward( const glm::vec3& target );

    const   ViewParams& ViewData( void ) const;
};

INLINE void InputCamera::Walk( float amount )
{


    glm::vec4 forward = viewData.inverseOrient * glm::vec4( 0.0f, 0.0f, -amount, 1.0f );

    viewData.forward = glm::vec3( forward );
    viewData.origin += viewData.forward;
}

INLINE void InputCamera::Strafe( float amount )
{
    glm::vec4 right = viewData.inverseOrient * glm::vec4( amount, 0.0f, 0.0f, 1.0f );

    viewData.right = glm::vec3( right );
    viewData.origin += viewData.right;
}

INLINE void InputCamera::Raise( float amount )
{
    glm::vec4 up = viewData.inverseOrient * glm::vec4( 0.0f, amount, 0.0f, 1.0f );

    viewData.right = glm::vec3( up );
    viewData.origin += viewData.right;
}

INLINE void InputCamera::SetForward( const glm::vec3& forward )
{
    glm::vec3 lookDir = forward - viewData.origin;
    glm::vec3 projected = lookDir;
    viewData.forward = lookDir;

    // Evaluate whether or not we've crossed into the YZ plane
    if ( glm::abs( lookDir.x ) < 0.0001f && glm::abs( lookDir.z ) < 0.0001f )
    {
        projected.x = 0.0f;
        projected = glm::normalize( projected );

        glm::vec3 up = glm::cross( glm::vec3( 1.0f, 0.0f, 0.0f ), projected );
        glm::vec3 right = -glm::cross( up, lookDir );

        viewData.up = glm::normalize( up );
        viewData.right = glm::normalize( right );
    }
    else // No, so we're in the XZ plane instead
    {
        projected.y = 0.0f;
        projected = glm::normalize( projected );

        glm::vec3 right = -glm::cross( glm::vec3( 0.0f, 1.0f, 0.0f ), projected );
        glm::vec3 up = glm::cross( right, lookDir );

        viewData.up = glm::normalize( up );
        viewData.right = glm::normalize( right );
    }

    viewData.forward = glm::normalize( viewData.forward );
}

INLINE const ViewParams& InputCamera::ViewData( void ) const
{
    return viewData;
}
