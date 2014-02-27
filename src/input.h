#pragma once

#include "common.h"
#include "eu_ang.h"

static const int KEY_COUNT = 8;

struct viewParams_t
{
    glm::vec3   forward;
    glm::vec3   up;
    glm::vec3   right;

    glm::vec3   origin;

    float       fovy, aspect, zNear, zFar;

    glm::mat4   transform;

    glm::mat4   orientation;
    glm::mat4   inverseOrient;

    glm::mat4   clipTransform;
};

class InputCamera
{
    viewParams_t    viewData;

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

    void    SetPerspective( float fovy, float aspect, float znear, float zfar );
    void    SetForward( const glm::vec3& target );

    glm::vec3   Forward( void ) const;
    glm::vec3   Up( void ) const;
    glm::vec3   Right( void ) const;

    const   viewParams_t& ViewData( void ) const;
};

INLINE glm::vec3 InputCamera::Forward( void ) const
{
    glm::vec4 forward = viewData.inverseOrient * glm::vec4( 0.0f, 0.0f, -1.0f, 1.0f );

    return glm::vec3( forward );
}

INLINE glm::vec3 InputCamera::Right( void ) const
{
    glm::vec4 right = viewData.inverseOrient * glm::vec4( 1.0f, 0.0f, 0.0f, 1.0f );

    return glm::vec3( right );
}

INLINE glm::vec3 InputCamera::Up( void ) const
{
    glm::vec4 up = viewData.inverseOrient * glm::vec4( 0.0f, 1.0f, 0.0f, 1.0f );

    return glm::vec3( up );
}

INLINE void InputCamera::Walk( float amount )
{
    viewData.forward = Forward() * amount;
    viewData.origin += viewData.forward;
}

INLINE void InputCamera::Strafe( float amount )
{
    viewData.right = Right() * amount;
    viewData.origin += viewData.right;
}

INLINE void InputCamera::Raise( float amount )
{
    viewData.right = Up() * amount;
    viewData.origin += viewData.right;
}

INLINE void InputCamera::SetPerspective( float fovy, float aspect, float zNear, float zFar )
{
    viewData.clipTransform = glm::perspective( fovy, aspect, zNear, zFar );

    // Cache params for frustum culling
    viewData.fovy = fovy;
    viewData.aspect = aspect;
    viewData.zNear = zNear;
    viewData.zFar = zFar;
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

INLINE const viewParams_t& InputCamera::ViewData( void ) const
{
    return viewData;
}
