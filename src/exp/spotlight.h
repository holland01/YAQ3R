#pragma once

#include "../common.h"

class SpotLight
{
    glm::mat4   worldTransform, surfaceTransform;

    GLuint      vbos[ NUM_SPOTLIGHT_VBOS ];

public:
    glm::vec3   position, direction;

    glm::vec4   color;

    float       intensity;

    SpotLight( void );

    ~SpotLight( void );

    void Draw( void );

    void Update( void );
};
