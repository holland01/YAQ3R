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
	glm::vec4 plane( glm::normalize( position ) );

	if ( !normalizeDistance )
	{
		plane.w = glm::dot( glm::vec3( origin ), glm::vec3( position ) );
	}

	return plane;
}

void Frustum::Update( const viewParams_t& view, bool normalizeDistance )
{
	mvp = view.clipTransform * view.transform;
	
#define _fetch_ glm::row
	glm::vec4 origin( _fetch_( mvp, 3 ) );

	frustPlanes[ FRUST_LEFT ].normal = CalcPlaneFromOrigin( origin + _fetch_( mvp, 0 ), origin, normalizeDistance );
	frustPlanes[ FRUST_RIGHT ].normal = CalcPlaneFromOrigin( origin - _fetch_( mvp, 0 ), origin, normalizeDistance );
	frustPlanes[ FRUST_BOTTOM ].normal = CalcPlaneFromOrigin( origin + _fetch_( mvp, 1 ), origin, normalizeDistance );
	frustPlanes[ FRUST_TOP ].normal = CalcPlaneFromOrigin( origin - _fetch_( mvp, 1 ), origin, normalizeDistance );
	frustPlanes[ FRUST_FAR ].normal = CalcPlaneFromOrigin( origin + _fetch_( mvp, 2 ), origin, normalizeDistance );
	frustPlanes[ FRUST_NEAR ].normal = CalcPlaneFromOrigin( origin - _fetch_( mvp, 2 ), origin, normalizeDistance );
#undef _fetch_
}

// Adding plane[ 3 ] ( which is the distance from the plane to the origin offsets the plane so we can ensure that the point is in front of the plane normal )
#define dist( point, plane ) \
    ( ( plane[ 0 ] ) * ( point[ 0 ] ) + ( plane[ 1 ] ) * ( point[ 1 ] ) + ( plane[ 2 ] ) * ( point[ 2 ] ) + plane[ 3 ] )

bool Frustum::IntersectsBox( const AABB& box ) const
{
#define C(v) ( ( v ) )

	std::array< glm::vec4, 8 > clipBounds = 
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
    for ( int i = 0; i < FRUST_NUM_PLANES; ++i )
    {
        if ( dist( clipBounds[ 0 ], frustPlanes[ i ].normal ) >= 0 ) continue;
        if ( dist( clipBounds[ 1 ], frustPlanes[ i ].normal ) >= 0 ) continue;
        if ( dist( clipBounds[ 2 ], frustPlanes[ i ].normal ) >= 0 ) continue;
        if ( dist( clipBounds[ 3 ], frustPlanes[ i ].normal ) >= 0 ) continue;
        if ( dist( clipBounds[ 4 ], frustPlanes[ i ].normal ) >= 0 ) continue;
        if ( dist( clipBounds[ 5 ], frustPlanes[ i ].normal ) >= 0 ) continue;
        if ( dist( clipBounds[ 6 ], frustPlanes[ i ].normal ) >= 0 ) continue;
        if ( dist( clipBounds[ 7 ], frustPlanes[ i ].normal ) >= 0 ) continue;

		rejectCount++;
        return false;
	}

	acceptCount++;
    return true;
}
