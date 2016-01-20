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

    std::unique_ptr< Program > prog;

    std::unique_ptr< InputCamera > camera;

    gTextureHandle_t texture;

    gTextureImage_t currImage;

    glm::vec2 invRowPitch;

    gVertexBufferHandle_t vbo;

    void Run( void );

public:

    TTextureTest( void );

    ~TTextureTest( void );

    void Load( void );
};

