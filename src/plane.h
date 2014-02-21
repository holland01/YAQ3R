#pragma once

#include "common.h"

/*
==============================================================

                        Plane.h

Equation Notes
-----------------

plane -> ax + by + cz + d = 0 -> ( ax + by + cz ) - ( ax1 + by1 + cz1 )
plane -> dot( p - p1, n ) = a( px - px1 ) + b( py - py1 ) + c( pz - pz1 )

d  -> -( ax1 + by1 + cz1 ) - the result of dot( p1, n ), via distribution.
n  -> ( a, b, c ) - the plane surface normal
p  -> ( x, y, z ) - arbitary point on the plane
p1 -> ( x1, y1, z1 ) - known point on the plane

d also is the "distance" from the origin to the plane, providing n is normalized.

==============================================================
*/

struct plane_t
{
    float       d;
    glm::vec3   points[ 3 ];
    glm::vec3   normal;
};

static INLINE void PlaneFrom3( plane_t* const outp, const glm::vec3& p, const glm::vec3& p1, const glm::vec3& p2 )
{
    const glm::vec3& e1 = p1 - p;
    const glm::vec3& e3 = p2 - p;

    outp->normal = glm::cross( e1, e3 );

    if ( glm::length( outp->normal ) > 1.0f )
        outp->normal = glm::normalize( outp->normal );

    outp->d = glm::dot( e1, outp->normal );

    outp->points[ 0 ] = p;
    outp->points[ 1 ] = p1;
    outp->points[ 2 ] = p2;
}

static INLINE float PointToPlaneDistance( const plane_t* const plane, const glm::vec3& v )
{
    const glm::vec3& p = v - plane->points[ 0 ];

    return glm::dot( p, plane->normal );
}




