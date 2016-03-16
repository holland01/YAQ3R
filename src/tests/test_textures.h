#pragma once

#include "test.h"
#include "glutil.h"
#include "renderer/texture.h"
#include "renderer/buffer.h"
#include "q3bsp.h"
#include "input.h"
#include <memory>

class TTextureTest : public Test
{
private:

	std::unique_ptr< Program > prog;

	std::unique_ptr< InputCamera > camera;

	gTextureHandle_t texture;

	gSamplerHandle_t sampler;

	gVertexBufferHandle_t vbo;

	Q3BspMap map;

	void SetupVertexData( void );

	void SetupProgram( void );

	void Run( void );

public:

	TTextureTest( void );

	~TTextureTest( void );

	void Load( void );
};

