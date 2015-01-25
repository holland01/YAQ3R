#pragma once

#include "common.h"
#include "vec.h"

#define BEZ_BUF_COUNT 2
#define BEZ_CONTROL_POINT_COUNT 9

struct bspVertex_t;

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

    void						Tesselate( int level );

    void						Render( void ) const;

	friend class				Q3BspMap;
};
