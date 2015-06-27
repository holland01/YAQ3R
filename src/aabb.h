#pragma once

#include "common.h"


/*
===========================================

        Axially-Aligned Bounding Box

===========================================
*/

class AABB
{
public:

    AABB( void ); // Calls Empty() on default init

    AABB( const glm::vec3& max, const glm::vec3& min );

    AABB( const AABB& toCopy );

    ~AABB( void );

    AABB&       operator =( AABB toAssign );

	bool		Encloses( const AABB& box ) const;

    void        Add( const glm::vec3& p );

    void        Empty( void ); // Sets maxPoint to -pseudoInfinity, and minPoint to pseudoInfinity

    void        TransformTo( const AABB& box, const glm::mat4& transform ); // Finds the smallest AABB from a given transformation

    glm::vec3       GetMaxRelativeToNormal( const glm::vec3& normal ) const;

    glm::vec3       GetMinRelativeToNormal( const glm::vec3 &normal ) const;

    glm::vec3        Center( void ) const;

    glm::vec3        Size( void ) const;

    glm::vec3        Radius( void ) const;

    glm::vec3        Corner( int index ) const;

	glm::vec4		Corner4( int index ) const;

    bool        IsEmpty( void ) const;

    bool        InPointRange( float k ) const;

    static void FromTransform( AABB& box, const glm::mat4& transform );

    static void FromPoints( AABB& box, const glm::vec3 p[], int32_t n );

    glm::vec3        maxPoint, minPoint;
};

INLINE glm::vec4 AABB::Corner4( int32_t index ) const
{
	return glm::vec4( Corner( index ), 1.0f );
}

INLINE bool	AABB::Encloses( const AABB& box ) const
{
#ifdef AABB_MAX_Z_LESS_THAN_MIN_Z
	
	//if ( minPoint.x > box.maxPoint.x ) return false;
	if ( minPoint.x > box.minPoint.x ) return false;
	//if ( maxPoint.x < box.minPoint.x ) return false;
	if ( maxPoint.x < box.maxPoint.x ) return false;

	//if ( minPoint.y > box.maxPoint.y ) return false;
	if ( minPoint.y > box.minPoint.y ) return false;
	//if ( maxPoint.y < box.minPoint.y ) return false;
	if ( maxPoint.y < box.maxPoint.y ) return false;

	//if ( minPoint.z < box.maxPoint.x ) return false;
	if ( minPoint.z < box.minPoint.z ) return false;
	//if ( maxPoint.z < box.minPoint.z ) return false;
	if ( maxPoint.z > box.maxPoint.z ) return false;

	return true;
#else
	return !glm::any( glm::greaterThan( minPoint, box.maxPoint ) ) && !glm::any( glm::lessThan( maxPoint, box.minPoint ) );
#endif
}