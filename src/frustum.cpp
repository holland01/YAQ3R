#include "frustum.h"
#include "input.h"
#include "aabb.h"
#include <array>

#define _DEBUG_FRUSTUM

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

#define F_CalcDist( plane ) ( ( plane ).d / glm::length( ( plane ).normal ) )
#define F_CalcNormal( a, b ) ( glm::cross( a, b ) )
void Frustum::Update( const viewParams_t& view, bool normalizeDistance )
{
	float tanHalfFovy = glm::tan( view.fovy * 0.5f );
	
	// We compute the reference angle since we want to base the sin/cosine on the angle from the x-axis;
	// without we have an angle from the z.
	float fov = glm::atan( view.aspect * 0.75f * tanHalfFovy );

	glm::vec3 u( view.inverseOrient[ 0 ] * glm::cos( fov ) );
	glm::vec3 v( view.inverseOrient[ 2 ] * -glm::sin( fov ) );
	glm::vec3 w( view.inverseOrient[ 1 ] );

	glm::vec3 planeLine( u + v );

	frustPlanes[ FRUST_RIGHT ].normal = F_CalcNormal( planeLine, -w );
	frustPlanes[ FRUST_RIGHT ].d = glm::dot( view.origin + planeLine, frustPlanes[ FRUST_RIGHT ].normal );

	planeLine = -u + v;

	frustPlanes[ FRUST_LEFT  ].normal = F_CalcNormal( planeLine, w );
	frustPlanes[ FRUST_LEFT  ].d = glm::dot( view.origin + planeLine, frustPlanes[ FRUST_LEFT ].normal );

	// Z is the initial axis for the horizontal planes
	fov = glm::atan( tanHalfFovy );
	
	u = -glm::vec3( view.inverseOrient[ 2 ] * glm::cos( fov ) );
	v = glm::vec3( view.inverseOrient[ 1 ] * -glm::sin( fov ) );
	w = glm::vec3( view.inverseOrient[ 0 ] );
	
	planeLine = -u + v;
	frustPlanes[ FRUST_TOP ].normal = F_CalcNormal( w, planeLine );
	frustPlanes[ FRUST_TOP ].d = glm::dot( view.origin + planeLine, frustPlanes[ FRUST_TOP ].normal ); 

	planeLine = u + v;
	frustPlanes[ FRUST_BOTTOM ].normal = F_CalcNormal( w, planeLine );
	frustPlanes[ FRUST_BOTTOM ].d = glm::dot( view.origin + planeLine, frustPlanes[ FRUST_BOTTOM ].normal );
}
#undef F_CalcNormal

// Adding plane[ 3 ] ( which is the distance from the plane to the origin ) offsets the plane so we can ensure that the point is in front of the plane normal

#ifdef _DEBUG_FRUSTUM
	static float F_PlaneSide( const glm::vec3& point, const plane_t& plane )
	{
		float x = glm::dot( point, plane.normal ) - plane.d;

		return x;
	}
#else
#	define F_PlaneSide( point, plane ) ( glm::dot( ( point ), ( plane ).normal ) - ( plane ).d
) 
#endif

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
        if ( F_PlaneSide( clipBounds[ 0 ], frustPlanes[ i ] ) >= 0 ) continue;
        if ( F_PlaneSide( clipBounds[ 1 ], frustPlanes[ i ] ) >= 0 ) continue;
        if ( F_PlaneSide( clipBounds[ 2 ], frustPlanes[ i ] ) >= 0 ) continue;
        if ( F_PlaneSide( clipBounds[ 3 ], frustPlanes[ i ] ) >= 0 ) continue;
        if ( F_PlaneSide( clipBounds[ 4 ], frustPlanes[ i ] ) >= 0 ) continue;
        if ( F_PlaneSide( clipBounds[ 5 ], frustPlanes[ i ] ) >= 0 ) continue;
        if ( F_PlaneSide( clipBounds[ 6 ], frustPlanes[ i ] ) >= 0 ) continue;
        if ( F_PlaneSide( clipBounds[ 7 ], frustPlanes[ i ] ) >= 0 ) continue;

		rejectCount++;
        return false;
	}

	acceptCount++;
    return true;
}

#ifdef F_PlaneSide
#	undef F_PlaneSide
#endif
