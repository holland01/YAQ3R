#pragma once

#include "common.h"
#include "vec.h"

#define BEZ_BUF_COUNT 3

class BezPatch
{
private:

    GLuint                      buffers[ BEZ_BUF_COUNT ];

    std::vector< vec3f_t >      vertices;

    std::vector< int  >         indices;
    std::vector< int  >         trisPerRow;
    std::vector< int* >         rowIndices;

public:

    int                     tessLevel;

    vec3f_t                 controlPoints[ 9 ];

    void                    Alloc( void );

    void                    Tesselate( void );

    void                    Render( void ) const;
};
