#include "common.h"

glm::vec3 GetBoundsCenter( glm::ivec3& max, glm::ivec3& min );

glm::fquat MakeQuat( float angRad, const glm::vec3& axis );

template < typename T >
INLINE T Inv255( void )
{
	return T( 0.0039215686274509803921568627451 );
}

template < typename T >
INLINE T Inv128( void )
{
	return T( 0.0078125 );
}

template < typename T >
INLINE T Inv64( void )
{
	return T( 0.015625 );
}

