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

void Frustum::Update( const viewParams_t& view )
{
	mvp = view.clipTransform * view.transform; 
	
	frustPlanes[ FRUST_LEFT ].normal = glm::normalize( glm::row( mvp, 3 ) + glm::row( mvp, 0 ) );
	frustPlanes[ FRUST_RIGHT ].normal = glm::normalize( glm::row( mvp, 3 ) - glm::row( mvp, 0 ) );
	frustPlanes[ FRUST_BOTTOM ].normal = glm::normalize( glm::row( mvp, 3 ) + glm::row( mvp, 1 ) );
	frustPlanes[ FRUST_TOP ].normal = glm::normalize( glm::row( mvp, 3 ) - glm::row( mvp, 1 ) );
	frustPlanes[ FRUST_FAR ].normal = glm::normalize( glm::row( mvp, 3 ) + glm::row( mvp, 2 ) ); 
	frustPlanes[ FRUST_NEAR ].normal = glm::normalize( glm::row( mvp, 3 ) - glm::row( mvp, 2 ) );

	mvp = glm::mat4( 1.0f );
}

// Adding plane[ 3 ] ( which is the distance from the plane to the origin offsets the plane so we can ensure that the point is in front of the plane normal )
#define dist( point, plane ) \
    ( ( plane[ 0 ] ) * ( point[ 0 ] ) + ( plane[ 1 ] ) * ( point[ 1 ] ) + ( plane[ 2 ] ) * ( point[ 2 ] ) + plane[ 3 ] )

bool Frustum::IntersectsBox( const AABB& box ) const
{
	std::array< glm::vec4, 8 > clipBounds = 
	{
		mvp * box.Corner4( 0 ),
		mvp * box.Corner4( 1 ),
		mvp * box.Corner4( 2 ),
		mvp * box.Corner4( 3 ),
		mvp * box.Corner4( 4 ),
		mvp * box.Corner4( 5 ),
		mvp * box.Corner4( 6 ),
		mvp * box.Corner4( 7 )
	};

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
