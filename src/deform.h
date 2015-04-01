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

void TessellateTri( 
	std::vector< bspVertex_t >& outVerts, 
	std::vector< triangle_t >& triIndices,
	const float amount, 
	const bspVertex_t& a, 
	const bspVertex_t& b, 
	const bspVertex_t& c
);
