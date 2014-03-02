#pragma once

#include "test.h"

struct tbezvert_t
{
    float3_t     vertex;
    float4_t   color;
};

#define TBEZ_LEVEL 4

class TBezSurface : public Test
{
private:

    std::vector< tbezvert_t > vertices;
    std::vector< int        > indices;

    glm::vec3 controlPoints[ TBEZ_LEVEL + 1 ][ TBEZ_LEVEL + 1 ];

    GLuint vbo;
    GLuint vao;

    GLuint program;

    void Run( void );

public:
    TBezSurface( void );

    ~TBezSurface( void );

    bool Load( void );
};
