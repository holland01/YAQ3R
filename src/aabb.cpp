#include "aabb.h"

/*
===========================================

        Axially-Aligned Bounding Box

===========================================
*/

AABB::AABB( void )
{
    Empty();
}

AABB::AABB( const glm::vec3& max, const glm::vec3& min )
    : maxPoint( max ),
      minPoint( min )
{
}

AABB::AABB( const AABB& toCopy )
    : maxPoint( toCopy.maxPoint ),
      minPoint( toCopy.minPoint )
{
}

AABB::~AABB( void )
{
}

AABB& AABB::operator =( AABB toAssign )
{
    maxPoint = toAssign.maxPoint;
    minPoint = toAssign.minPoint;

    return *this;
}

void AABB::Add( const glm::vec3& p )
{
    if ( p.x < minPoint.x ) minPoint.x = p.x;
    if ( p.y < minPoint.y ) minPoint.y = p.y;
    if ( p.z < minPoint.z ) minPoint.z = p.z;

    if ( p.x > maxPoint.x ) maxPoint.x = p.x;
    if ( p.y > maxPoint.y ) maxPoint.y = p.x;
    if ( p.z > maxPoint.z ) maxPoint.z = p.z;
}

void AABB::Empty( void )
{
    const float pseudoInfinity = 1e37f;

    maxPoint = glm::vec3( -pseudoInfinity );
    minPoint = glm::vec3( pseudoInfinity );
}

void AABB::TransformTo( const AABB& box, const glm::mat4& transform )
{
    maxPoint = minPoint = glm::vec3( transform[ 3 ] );

    for ( int32_t i = 0; i < 3; ++i )
    {
        float px = transform[ i ][ 0 ];
        float py = transform[ i ][ 1 ];
        float pz = transform[ i ][ 2 ];

        // Scale each basis' respective coordinate
        // along its respective axis for our max/min coordinates.
        // If pn (where 'n' is { x | y | z }) is > 0, compute box's point
        // in parallel with this one's. Otherwise, compute this instances
        // points opposite to that of the box's, in an effort to 'shrink'
        // our current points.

        if ( px > 0.0f )
        {
            minPoint.x += box.minPoint.x * px;
            maxPoint.x += box.maxPoint.x * px;
        }
        else
        {
            minPoint.x += box.maxPoint.x * px;
            maxPoint.x += box.minPoint.x * px;
        }

        if ( py > 0.0f )
        {
            minPoint.y += box.minPoint.y * py;
            maxPoint.y += box.maxPoint.y * py;
        }
        else
        {
            minPoint.y += box.maxPoint.y * py;
            maxPoint.y += box.minPoint.y * py;
        }

        if ( pz > 0.0f )
        {
            minPoint.z += box.minPoint.z * pz;
            maxPoint.z += box.maxPoint.z * pz;
        }
        else
        {
            minPoint.z += box.maxPoint.z * pz;
            maxPoint.z += box.minPoint.z * pz;
        }
    }
}

glm::vec3 AABB::GetMaxRelativeToNormal( const glm::vec3 &normal ) const
{
    glm::vec3 p( minPoint.x, minPoint.y, minPoint.z );

    if ( normal.x >= 0 )
        p.x = maxPoint.x;

    if ( normal.y >= 0 )
        p.y = maxPoint.y;

    if ( normal.z >= 0 )
        p.z = maxPoint.z;

    return p;
}

glm::vec3 AABB::GetMinRelativeToNormal( const glm::vec3 &normal ) const
{
    glm::vec3 n( maxPoint.x, maxPoint.y, maxPoint.z );

    if ( normal.x >= 0 )
        n.x = minPoint.x;

    if ( normal.y >= 0 )
        n.y = minPoint.y;

    if ( normal.z >= 0 )
        n.z = minPoint.z;

    return n;
}


glm::vec3 AABB::Center( void ) const
{
    const glm::vec3& p = ( maxPoint + minPoint );

    return p / 2.0f;
}

glm::vec3 AABB::Size( void ) const
{
    return maxPoint - minPoint;
}

glm::vec3 AABB::Radius( void ) const
{
    return maxPoint - Center();
}

glm::vec3 AABB::Corner( int32_t index ) const
{
    assert( index >= 0 );
    assert( index <= 7 );

    return glm::vec3(
        ( index & 1 ) ? maxPoint.x : minPoint.x,
        ( index & 2 ) ? maxPoint.y : minPoint.y,
        ( index & 4 ) ? maxPoint.z : minPoint.z
    );
}

bool AABB::IsEmpty( void ) const
{
    // Check to see if any of our
    // axes are inverted

    return ( maxPoint.x < minPoint.x )
        || ( maxPoint.y < minPoint.y )
        || ( maxPoint.z < minPoint.z );
}

bool AABB::InPointRange( float k ) const
{
    return ( maxPoint.x >= k && maxPoint.y >= k && maxPoint.z >= k )
        && ( minPoint.x <= k && minPoint.y <= k && minPoint.z <= k );
}

static const float AABB_SIZE_FACTOR = 1.5f;

void AABB::FromTransform( AABB &box, const glm::mat4 &transform )
{
    // Compute our AABB using -

    // iter->world's scaling:
    float sx, sy, sz;

    sx = transform[ 0 ][ 0 ] * AABB_SIZE_FACTOR; // ensure our AABB is just *slightly* larger than our object.
    sy = transform[ 1 ][ 1 ] * AABB_SIZE_FACTOR;
    sz = transform[ 2 ][ 2 ] * AABB_SIZE_FACTOR;

    // and iter->world's translation:
    float tx, ty, tz;

    tx = transform[ 3 ][ 0 ];
    ty = transform[ 3 ][ 1 ];
    tz = transform[ 3 ][ 2 ];

    box.maxPoint.x = tx + sx;
    box.maxPoint.y = ty + sy;
    box.maxPoint.z = tz + sz;

    box.minPoint.x = tx - sx;
    box.minPoint.y = ty - sy;
    box.minPoint.z = tz - sz;
}

void AABB::FromPoints( AABB& box, const glm::vec3 v[], int32_t n )
{
    for ( int32_t i = 0; i < n; ++i )
    {
        box.Add( v[ i ] );
    }
}
