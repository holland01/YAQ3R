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
    float hOver2 = tan( view.fovy / 2.0f ); // height / 2
    float wOver2 = view.aspect * hOver2; // width / 2

    const glm::vec3& right = view.right * wOver2 * view.zFar;
    const glm::vec3& top   = view.up * hOver2 * view.zFar;
    const glm::vec3& center = view.origin + view.forward * view.zFar;

    const glm::vec3& flt = center - right + top; // far left top
    const glm::vec3& frt = center + right + top; // far right top
    const glm::vec3& flb = center - right - top; // far left bottom
    const glm::vec3& frb = center + right - top; // far right bottom

    float nearScale = glm::normalize( ( 1.0f / view.zFar ) * view.zNear );

    const glm::vec3& nlt = ( center - right + top ) * nearScale; // near left top
    const glm::vec3& nrt = ( center + right + top ) * nearScale; // near right top
    const glm::vec3& nlb = ( center - right - top ) * nearScale; // near left bottom
    const glm::vec3& nrb = ( center + right - top ) * nearScale; // near right bottom

    // Update our frustum planes
    PlaneFrom3( &frustPlanes[ FRUST_NEAR ], nlt, nrt, nlb );
    PlaneFrom3( &frustPlanes[ FRUST_FAR ], flt, frt, flb );

    PlaneFrom3( &frustPlanes[ FRUST_LEFT ], flt, flb, nlb );
    PlaneFrom3( &frustPlanes[ FRUST_RIGHT ], frt, frb, nrb );

    PlaneFrom3( &frustPlanes[ FRUST_TOP ], nrt, nlt, frt );
    PlaneFrom3( &frustPlanes[ FRUST_BOTTOM ], nrb, nlb, frb );
}

bool Frustum::IntersectsBox( const AABB& box )
{
    for ( int i = 0; i < FRUST_NUM_PLANES; ++i )
    {
        int out = 0;

        if ( glm::dot( box.Corner( 0 ), frustPlanes[ i ].normal ) + frustPlanes[ i ].d < 0 ) out++;
        if ( glm::dot( box.Corner( 1 ), frustPlanes[ i ].normal ) + frustPlanes[ i ].d < 0 ) out++;
        if ( glm::dot( box.Corner( 2 ), frustPlanes[ i ].normal ) + frustPlanes[ i ].d < 0 ) out++;
        if ( glm::dot( box.Corner( 3 ), frustPlanes[ i ].normal ) + frustPlanes[ i ].d < 0 ) out++;
        if ( glm::dot( box.Corner( 4 ), frustPlanes[ i ].normal ) + frustPlanes[ i ].d < 0 ) out++;
        if ( glm::dot( box.Corner( 5 ), frustPlanes[ i ].normal ) + frustPlanes[ i ].d < 0 ) out++;
        if ( glm::dot( box.Corner( 6 ), frustPlanes[ i ].normal ) + frustPlanes[ i ].d < 0 ) out++;
        if ( glm::dot( box.Corner( 7 ), frustPlanes[ i ].normal ) + frustPlanes[ i ].d < 0 ) out++;

        if ( out == 8 )
            return false;
    }

    return true;
}
