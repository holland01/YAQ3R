#pragma once

#include "test.h"
#include "glutil.h"
#include "renderer/texture.h"
#include "renderer/buffer.h"
#include "input.h"
#include <memory>

class TTextureTest : public Test
{
private:

    GLuint vao;

    std::unique_ptr< Program > prog;

    std::unique_ptr< InputCamera > camera;

    gTextureHandle_t texture;

    glm::vec4 imageTransform;

    gVertexBufferHandle_t vbo;

    void Run( void );

public:

    TTextureTest( void );

    ~TTextureTest( void );

    void Load( void );

    void OnKeyPress( int key, int scancode, int action, int mods );
};

