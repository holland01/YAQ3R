#include "frustum.h"
#include "input.h"
#include "aabb.h"
#include <array>

Frustum::Frustum( void )
	:	acceptCount( 0 ),
		rejectCount( 0 ),
		mvp( 1.0f )
{
    memset( frustPlanes, 0, sizeof( plane_t ) * FRUST_NUM_PLANES );
}

Frustum::~Frustum( void )
{
}

glm::vec4 Frustum::CalcPlaneFromOrigin( const glm::vec4& position, const glm::vec4& origin, bool normalizeDistance )
{
	glm::vec4 plane( 0.0f );
	plane.x = position.x;
	plane.y = position.y;
	plane.z = position.z;
	plane	= glm::normalize( plane );
	plane.w = glm::dot( glm::vec3( origin ), glm::vec3( position ) ); 

	return plane;
}

void Frustum::Update( const viewParams_t& view, bool normalizeDistance )
{
	float tanHalfFovy = glm::tan( view.fovy * 0.5f );
	
	// We compute the reference angle since we want to base the sin/cosine on the angle from the x-axis;
	// without we have an angle from the z.
	float fov = glm::half_pi< float >() - glm::atan( view.aspect * 0.75f * tanHalfFovy );
	
	glm::vec3 u( view.transform[ 0 ] * glm::cos( fov ) );
	glm::vec3 v( view.transform[ 2 ] * glm::sin( fov ) );
	glm::vec3 w( view.transform[ 1 ] );

	// figure out the issue with the plane distance. Also, the bottom frustum normal is just (0, 0, 1)
	// initially, which doesn't make much sense...

	frustPlanes[ FRUST_RIGHT ].normal = glm::normalize( glm::cross( w, u + v ) );
	frustPlanes[ FRUST_RIGHT ].d = glm::dot( /*u + v*/, frustPlanes[ FRUST_RIGHT ].normal );

	frustPlanes[ FRUST_LEFT  ].normal = glm::normalize( glm::cross( -u + v, w ) );
	frustPlanes[ FRUST_LEFT  ].d = glm::dot( -u + v, frustPlanes[ FRUST_LEFT ].normal );

	// Z is the initial axis for the horizontal planes
	fov = glm::atan( tanHalfFovy );
	
	u = glm::vec3( view.transform[ 2 ] * glm::cos( fov ) );
	v = glm::vec3( view.transform[ 1 ] * glm::sin( fov ) );
	w = glm::vec3( view.transform[ 0 ] );

	frustPlanes[ FRUST_TOP ].normal = -glm::normalize( glm::cross( w, u + v ) );
	frustPlanes[ FRUST_TOP ].d = glm::dot( u + v, frustPlanes[ FRUST_TOP ].normal ); 

	frustPlanes[ FRUST_BOTTOM ].normal = -glm::normalize( glm::cross( -w, -v + u ) );
	frustPlanes[ FRUST_BOTTOM ].d = glm::dot( -v + u, frustPlanes[ FRUST_BOTTOM ].normal );
}

// Adding plane[ 3 ] ( which is the distance from the plane to the origin ) offsets the plane so we can ensure that the point is in front of the plane normal
#define dist( point, plane ) ( glm::dot( ( point ), ( plane ).normal ) + ( plane ).d ) 

bool Frustum::IntersectsBox( const AABB& box ) const
{
#define C(v) ( glm::vec3( ( v ) ) )

	std::array< glm::vec3, 8 > clipBounds = 
	{
		C( box.Corner4( 0 ) ),
		C( box.Corner4( 1 ) ),
		C( box.Corner4( 2 ) ),
		C( box.Corner4( 3 ) ),
		C( box.Corner4( 4 ) ),
		C( box.Corner4( 5 ) ),
		C( box.Corner4( 6 ) ),
		C( box.Corner4( 7 ) )
	};
#undef C

	// Test each corner against every plane normal
    for ( int i = 0; i < 4; ++i )
    {
        if ( dist( clipBounds[ 0 ], frustPlanes[ i ] ) >= 0 ) continue;
        if ( dist( clipBounds[ 1 ], frustPlanes[ i ] ) >= 0 ) continue;
        if ( dist( clipBounds[ 2 ], frustPlanes[ i ] ) >= 0 ) continue;
        if ( dist( clipBounds[ 3 ], frustPlanes[ i ] ) >= 0 ) continue;
        if ( dist( clipBounds[ 4 ], frustPlanes[ i ] ) >= 0 ) continue;
        if ( dist( clipBounds[ 5 ], frustPlanes[ i ] ) >= 0 ) continue;
        if ( dist( clipBounds[ 6 ], frustPlanes[ i ] ) >= 0 ) continue;
        if ( dist( clipBounds[ 7 ], frustPlanes[ i ] ) >= 0 ) continue;

		rejectCount++;
        return false;
	}

	acceptCount++;
    return true;
}
