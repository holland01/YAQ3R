#pragma once

#include "test.h"

struct tbezvert_t
{
    vec3f_t     vertex;
    color4f_t   color;
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

    void Load( void );
};
