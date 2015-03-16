#pragma once

#include "common.h"
#include "vec.h"
#include "q3m_model.h"

#define BEZ_BUF_COUNT 2
#define BEZ_CONTROL_POINT_COUNT 9

struct bspVertex_t;
struct bspFace_t;
struct shaderInfo_t;
struct deformModel_t;
struct mapModel_t;
struct mapData_t;

class BezPatch
{
public:
    GLuint                      vbo;

    std::vector< bspVertex_t >  vertices;

	mutable size_t lastCount;

    std::vector< int >          indices;
	std::vector< int* >			rowIndices;
	std::vector< int >			trisPerRow;

	int							subdivLevel;

    const bspVertex_t*			controlPoints[ BEZ_CONTROL_POINT_COUNT ];

	BezPatch( void );
	~BezPatch( void );

    void						Tessellate( int level );

    void						Render( void ) const;
};

struct baryCoordSystem_t
{
	glm::vec3 a;

	glm::vec3 v0;
	glm::vec3 v1;
	
	float d00;
	float d01;
	float d11;

	float D;

	baryCoordSystem_t( const glm::vec3& va, const glm::vec3& vb, const glm::vec3& vc )
		: a( va ),
		  v0( vb - a ), v1( vc - a ),
		  d00( glm::dot( v0, v0 ) ),
		  d01( glm::dot( v0, v1 ) ),
		  d11( glm::dot( v1, v1 ) ),
		  D( 1.0f / ( d00 * d11 - d01 * d01 ) )
	{	
	}

	~baryCoordSystem_t( void )
	{
	}

	glm::vec3 ToBaryCoords( const glm::vec3& p ) const
	{
		glm::vec3 v2( p - a );
		
		float d20 = glm::dot( v2, v0 );
		float d21 = glm::dot( v2, v1 );

		glm::vec3 b;
		b.x = ( d11 * d20 - d01 * d21 ) * D;
		b.y = ( d00 * d21 - d01 * d20 ) * D;
		b.z = 1.0f - b.x - b.y;

		return b;
	}

	bool IsInTri( const glm::vec3& p ) const
	{
		glm::vec3 bp( ToBaryCoords( p ) );

		return ( 0.0f <= bp.x && bp.x <= 1.0f ) 
			&& ( 0.0f <= bp.y && bp.y <= 1.0f )
			&& ( 0.0f <= bp.z && bp.z <= 1.0f );
	}
};

template < typename vertexType_t > static INLINE void TessellateTri( 
	std::vector< vertexType_t >& outVerts, 
	std::vector< triangle_t >& triIndices,
	std::function< vertexType_t( const glm::vec3& ) > vertexGenFn,
	const float amount, 
	const glm::vec3& a, 
	const glm::vec3& b, 
	const glm::vec3& c
);

#include "deform.inl"
