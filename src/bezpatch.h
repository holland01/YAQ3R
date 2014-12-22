#pragma once

#include "common.h"
#include "vec.h"

#define BEZ_BUF_COUNT 2
#define BEZ_CONTROL_POINT_COUNT 9

struct bspVertex_t;

class BezPatch
{
public:
    GLuint                      buffers[ BEZ_BUF_COUNT ];

	GLuint						vao;

    std::vector< bspVertex_t >  vertices;

    std::vector< int  >         indices;

    const bspVertex_t*			controlPoints[ BEZ_CONTROL_POINT_COUNT ];

	GLuint						clientVbo, clientVao;

	BezPatch( GLuint clientVbo, GLuint clientVao );
	~BezPatch( void );

    void						Tesselate( int level );

    void						Render( void ) const;

	friend class				Q3BspMap;
};
