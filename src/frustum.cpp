#include "frustum.h"
#include "input.h"
#include "aabb.h"

Frustum::Frustum( void )
{
    //memset( frustPlanes, 0, sizeof( plane_t ) * FRUST_NUM_PLANES );
}

Frustum::~Frustum( void )
{
}

void Frustum::Update( const viewParams_t& view )
{
    const glm::mat4& clipSpace = view.clipTransform * view.transform;

    frustPlanes[ FRUST_LEFT ].normal[ 0 ] = clipSpace[ 0 ][ 3 ] - clipSpace[ 0 ][ 0 ];
    frustPlanes[ FRUST_LEFT ].normal[ 1 ] = clipSpace[ 1 ][ 3 ] - clipSpace[ 1 ][ 0 ];
    frustPlanes[ FRUST_LEFT ].normal[ 2 ] = clipSpace[ 2 ][ 3 ] - clipSpace[ 2 ][ 0 ];
    frustPlanes[ FRUST_LEFT ].normal[ 3 ] = clipSpace[ 3 ][ 3 ] - clipSpace[ 3 ][ 0 ];

    frustPlanes[ FRUST_RIGHT ].normal[ 0 ] = clipSpace[ 0 ][ 3 ] + clipSpace[ 0 ][ 0 ];
    frustPlanes[ FRUST_RIGHT ].normal[ 1 ] = clipSpace[ 1 ][ 3 ] + clipSpace[ 1 ][ 0 ];
    frustPlanes[ FRUST_RIGHT ].normal[ 2 ] = clipSpace[ 2 ][ 3 ] + clipSpace[ 2 ][ 0 ];
    frustPlanes[ FRUST_RIGHT ].normal[ 3 ] = clipSpace[ 3 ][ 3 ] + clipSpace[ 3 ][ 0 ];

    frustPlanes[ FRUST_BOTTOM ].normal[ 0 ] = clipSpace[ 0 ][ 3 ] - clipSpace[ 0 ][ 1 ];
    frustPlanes[ FRUST_BOTTOM ].normal[ 1 ] = clipSpace[ 1 ][ 3 ] - clipSpace[ 1 ][ 1 ];
    frustPlanes[ FRUST_BOTTOM ].normal[ 2 ] = clipSpace[ 2 ][ 3 ] - clipSpace[ 2 ][ 1 ];
    frustPlanes[ FRUST_BOTTOM ].normal[ 3 ] = clipSpace[ 3 ][ 3 ] - clipSpace[ 3 ][ 1 ];

    frustPlanes[ FRUST_TOP ].normal[ 0 ] = clipSpace[ 0 ][ 3 ] + clipSpace[ 0 ][ 1 ];
    frustPlanes[ FRUST_TOP ].normal[ 1 ] = clipSpace[ 1 ][ 3 ] + clipSpace[ 1 ][ 1 ];
    frustPlanes[ FRUST_TOP ].normal[ 2 ] = clipSpace[ 2 ][ 3 ] + clipSpace[ 2 ][ 1 ];
    frustPlanes[ FRUST_TOP ].normal[ 3 ] = clipSpace[ 3 ][ 3 ] + clipSpace[ 3 ][ 1 ];

    frustPlanes[ FRUST_NEAR ].normal[ 0 ] = clipSpace[ 0 ][ 3 ] + clipSpace[ 0 ][ 2 ];
    frustPlanes[ FRUST_NEAR ].normal[ 1 ] = clipSpace[ 1 ][ 3 ] + clipSpace[ 1 ][ 2 ];
    frustPlanes[ FRUST_NEAR ].normal[ 2 ] = clipSpace[ 2 ][ 3 ] + clipSpace[ 2 ][ 2 ];
    frustPlanes[ FRUST_NEAR ].normal[ 3 ] = clipSpace[ 3 ][ 3 ] + clipSpace[ 3 ][ 2 ];

    frustPlanes[ FRUST_FAR ].normal[ 0 ] = clipSpace[ 0 ][ 3 ] - clipSpace[ 0 ][ 2 ];
    frustPlanes[ FRUST_FAR ].normal[ 1 ] = clipSpace[ 1 ][ 3 ] - clipSpace[ 1 ][ 2 ];
    frustPlanes[ FRUST_FAR ].normal[ 2 ] = clipSpace[ 2 ][ 3 ] - clipSpace[ 2 ][ 2 ];
    frustPlanes[ FRUST_FAR ].normal[ 3 ] = clipSpace[ 3 ][ 3 ] - clipSpace[ 3 ][ 2 ];
}

#define dist( point, plane ) \
    ( ( plane[ 0 ] ) * ( point[ 0 ] ) + ( plane[ 1 ] ) * ( point[ 1 ] ) + ( plane[ 2 ] ) * ( point[ 2 ] ) + ( plane[ 3 ] ) )

bool Frustum::IntersectsBox( const AABB& box )
{
    for ( int i = 0; i < FRUST_NUM_PLANES; ++i )
    {
        int out = 0;

        if ( dist( box.Corner( 0 ), frustPlanes[ i ].normal ) < 0 ) out++;
        if ( dist( box.Corner( 1 ), frustPlanes[ i ].normal ) < 0 ) out++;
        if ( dist( box.Corner( 2 ), frustPlanes[ i ].normal ) < 0 ) out++;
        if ( dist( box.Corner( 3 ), frustPlanes[ i ].normal ) < 0 ) out++;
        if ( dist( box.Corner( 4 ), frustPlanes[ i ].normal ) < 0 ) out++;
        if ( dist( box.Corner( 5 ), frustPlanes[ i ].normal ) < 0 ) out++;
        if ( dist( box.Corner( 6 ), frustPlanes[ i ].normal ) < 0 ) out++;
        if ( dist( box.Corner( 7 ), frustPlanes[ i ].normal ) < 0 ) out++;

        if ( out == 8 )
            return false;
    }

    return true;
}
