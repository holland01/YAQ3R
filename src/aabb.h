#pragma once

#include "common.h"
/*
===========================================

		Axially-Aligned Bounding Box

===========================================
*/

struct plane_t;

class AABB
{
public:

	enum face_t
	{
		FACE_TOP = 0,
		FACE_RIGHT,
		FACE_FRONT,
		FACE_LEFT,
		FACE_BACK,
		FACE_BOTTOM
	};

	AABB( void ); // Calls Empty() on default init

	AABB( const glm::vec3& max, const glm::vec3& min );

	AABB( const AABB& toCopy );

	~AABB( void );

	AABB&       operator =( AABB toAssign );

	bool		Encloses( const glm::vec3& point ) const;

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

	bool			InXRange( const glm::vec3& v ) const;

	bool			InYRange( const glm::vec3& v ) const;

	bool			InZRange( const glm::vec3& v ) const;

	bool			IsEmpty( void ) const;

	bool			InPointRange( float k ) const;

	float			CalcIntersection( const glm::vec3& ray, const glm::vec3& origin ) const;

	void			GetFacePlane( face_t face, plane_t& plane ) const;

	static void		FromTransform( AABB& box, const glm::mat4& transform );

	static void		FromPoints( AABB& box, const glm::vec3 p[], int32_t n );

	glm::vec3 maxPoint, minPoint;
};

INLINE glm::vec4 AABB::Corner4( int32_t index ) const
{
	return glm::vec4( Corner( index ), 1.0f );
}

INLINE bool AABB::Encloses( const glm::vec3& point ) const
{
	if ( minPoint.x > point.x || maxPoint.x < point.x ) return false;
	if ( minPoint.y > point.y || maxPoint.y < point.y ) return false;
#ifdef AABB_MAX_Z_LESS_THAN_MIN_Z
	if ( maxPoint.z > point.z || minPoint.z < point.z ) return false;
#else
	if ( minPoint.z > point.z || maxPoint.z < point.z ) return false;
#endif

	return true;
}

INLINE bool	AABB::Encloses( const AABB& box ) const
{
#ifdef AABB_MAX_Z_LESS_THAN_MIN_Z

	if ( minPoint.x > box.minPoint.x ) return false;
	if ( maxPoint.x < box.maxPoint.x ) return false;

	if ( minPoint.y > box.minPoint.y ) return false;
	if ( maxPoint.y < box.maxPoint.y ) return false;

	if ( minPoint.z < box.minPoint.z ) return false;
	if ( maxPoint.z > box.maxPoint.z ) return false;

	return true;
#else
	return !glm::any( glm::greaterThan( minPoint, box.maxPoint ) ) && !glm::any( glm::lessThan( maxPoint, box.minPoint ) );
#endif
}

INLINE bool	AABB::InXRange( const glm::vec3& v ) const
{
	return ( v.x <= maxPoint.x && v.x >= minPoint.x );
}

INLINE bool AABB::InYRange( const glm::vec3& v ) const
{
	return ( v.y <= maxPoint.y && v.y >= minPoint.y );
}

INLINE bool AABB::InZRange( const glm::vec3& v ) const
{
#ifdef AABB_MAX_Z_LESS_THAN_MIN_Z
	return ( v.z >= maxPoint.z && v.z <= minPoint.z );
#else
	return ( v.z <= maxPoint.z && v.z >= minPoint.z );
#endif

}
