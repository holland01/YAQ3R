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

	viewParams_t( void )
		: forward( 0.0f ), up( 0.0f ), right( 0.0f ),
		  origin( 0.0f ),
		  fovy( 0.0f ), aspect( 0.0f ), zNear( 0.0f ), zFar( 0.0f ),
		  transform( 1.0f ),
		  orientation( 1.0f ),
		  inverseOrient( 1.0f ),
		  clipTransform( 1.0f )
	{
	}
};

class InputCamera
{
    viewParams_t    viewData;

    EuAng           currRot, lastRot;

    glm::vec3       lastMouse;

    byte            keysPressed[ KEY_COUNT ];

public:

    InputCamera( void );

	InputCamera( const viewParams_t& view, const EuAng& currRot );

	InputCamera( const glm::mat4& view, const glm::mat4& projection );

	float	moveStep;

    void    EvalKeyPress( int key );
    void    EvalKeyRelease( int key );
    void    EvalMouseMove( float x, float y );

    void    Update( void );

    void    Walk( float amount );
    void    Strafe( float amount );
    void    Raise( float amount );

    void    SetPerspective( float fovy, float aspect, float znear, float zfar );
    void	SetClipTransform( const glm::mat4& proj );
	void	SetViewTransform( const glm::mat4& view );

	void	SetViewOrigin( const glm::vec3& origin );

    glm::vec3   Forward( void ) const;
    glm::vec3   Up( void ) const;
    glm::vec3   Right( void ) const;

    const   viewParams_t& ViewData( void ) const;
	viewParams_t& ViewDataMut( void );

	static viewParams_t CalcViewData(  );

	friend class Test;
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
	fovy = glm::radians( fovy );

    viewData.clipTransform = glm::perspective( fovy, aspect, zNear, zFar );

    // Cache params for frustum culling
    viewData.fovy = fovy;
    viewData.aspect = aspect;
    viewData.zNear = zNear;
    viewData.zFar = zFar;
}

INLINE void InputCamera::SetClipTransform( const glm::mat4& proj )
{
	viewData.clipTransform = proj;
}

INLINE void InputCamera::SetViewTransform( const glm::mat4& view )
{
    viewData.transform = view;
}

INLINE void InputCamera::SetViewOrigin( const glm::vec3& origin )
{
    viewData.origin = origin;
}

INLINE const viewParams_t& InputCamera::ViewData( void ) const
{
    return viewData;
}

INLINE viewParams_t& InputCamera::ViewDataMut( void )
{
    return viewData;
}
