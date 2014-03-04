#pragma once

#include "test.h"
#include "../input.h"
#include "../extern/tiny_obj_loader.h"

using namespace tinyobj;

struct objVertex_t
{
    float3_t position;
    float3_t normal;
    float4_t color;
};

struct objMesh_t
{
    objVertex_t*    vertices;
    int             numVertices;

    GLuint*         indices;
    int             numIndices;

    GLuint          vbos[ 2 ];
    GLuint          vao;

    glm::mat4       localTransform;
};

class KeyMover;

struct pointLight_t
{
    float       radius;

    float       specularStrength;
    float       specularShininess;

    glm::vec3   worldPos;
    glm::vec4   intensity;
    glm::vec4   ambient;
    glm::vec4   diffuse;


    GLuint      vao, vbo;
    GLuint      program;
    GLsizei     modelNumVertices;

    KeyMover*   mover;
    bool        drawLight;
};

class TLighting : public Test
{
private:

    static const int            NUM_BUFS_PER_MESH = 2;

    std::vector< objMesh_t >    meshes;

    Camera*                     camera;

    pointLight_t                light;

    GLuint                      program;
    GLuint                      vao;

    void        Run( void );

    void        InitLight( void );

    glm::vec4   CompLightPos( void ) const;

    void        DrawLight( const glm::vec4& lightWorldSpace ) const;

    void        DrawModel( const glm::vec4& lightWorldSpace ) const;

    void        ApplyModelToCameraTransform( GLuint program, const glm::mat4& model ) const;

    void        OnKeyPress( int key, int scancode, int action, int mods );

public:

    TLighting( void );

    ~TLighting( void );

    bool Load( void );
};


void FreePointLight( pointLight_t* light );
