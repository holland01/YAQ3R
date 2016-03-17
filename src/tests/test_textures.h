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

	std::unique_ptr< Program > atlasProg, textureProg;

	std::unique_ptr< InputCamera > camera;

	gTextureHandle_t texture;

	gSamplerHandle_t sampler;

	gVertexBufferHandle_t atlasQuad, textureQuad;

	Q3BspMap map;

	bool drawAtlas;

	uint8_t imageIndex;

	void SetupVertexData( void );

	void SetupProgram( void );

	Program* MakeProgram( const std::string& vertex, const std::string& fragment,
						  const std::vector< std::string >& additionalUnifs = std::vector< std::string >() );

	gVertexBufferHandle_t MakeQuadVbo( float width, float height );

	void Draw( Program& program, gVertexBufferHandle_t vbo );

	void Run( void );

public:

	TTextureTest( void );

	~TTextureTest( void );

	void Load( void );

	void OnInputEvent( SDL_Event* e );
};

