#pragma once

#include "texture.h"
#include "glutil.h"

void GU_SetupTexParams( const Program& program,
						const char* uniformPrefix,
						gTextureHandle_t texHandle,
						int32_t textureIndex,
						int32_t offset );


static INLINE void GU_ClearDepth( float d )
{
#ifdef EMSCRIPTEN
	GL_CHECK( glClearDepthf( d ) );
#else
	GL_CHECK( glClearDepth( d ) );
#endif
}

using guOffset_t = intptr_t;
using guBufferOffsetList_t = std::vector< guOffset_t >;
using guBufferRangeList_t = std::vector< GLsizei >;

static INLINE void GU_DrawElements( GLenum mode, guOffset_t buffOffset, GLsizei numIndices )
{
	GL_CHECK( glDrawElements( mode, numIndices, GL_UNSIGNED_INT, ( const GLvoid* )( buffOffset * G_INDEX_BYTE_STRIDE ) ) );
}

static INLINE void GU_MultiDrawElements( GLenum mode, const guBufferOffsetList_t& indexBuffers, const guBufferRangeList_t& indexBufferSizes )
{
	for ( uint32_t i = 0; i < indexBuffers.size(); ++i )
	{
		GU_DrawElements( mode, indexBuffers[ i ], indexBufferSizes[ i ] );
	}
}

class Q3BspMap;

void GU_LoadShaderTextures( Q3BspMap& map, gSamplerHandle_t sampler );

void GU_LoadMainTextures( Q3BspMap& map, gSamplerHandle_t sampler );

void GU_LoadStageTexture( glm::ivec2& maxDims, std::vector< gImageParams_t >& images,
	shaderInfo_t& info, int i, const gSamplerHandle_t& sampler );

using guImmPosList_t = std::vector< glm::vec3 >;

#ifndef EMSCRIPTEN
void GU_ImmLoadMatrices( const glm::mat4& view, const glm::mat4& proj );

void GU_ImmBegin( GLenum mode, const glm::mat4& view, const glm::mat4& proj );

void GU_ImmLoad( const guImmPosList_t& v, const glm::vec4& color );

void GU_ImmLoad( const guImmPosList_t& v, const glm::u8vec4& color );

void GU_ImmEnd( void );

void GU_ImmDrawLine( const glm::vec3& origin,
					 const glm::vec3& dir,
					 const glm::vec4& color,
					 const glm::mat4& view,
					 const glm::mat4& proj );
#endif

struct pushBlend_t
{
	GLint srcAlpha, srcRGB, destAlpha, destRGB;

	pushBlend_t( GLenum newSrc, GLenum newDst )
	{
		GL_CHECK( glGetIntegerv( GL_BLEND_SRC_ALPHA, &srcAlpha ) );
		GL_CHECK( glGetIntegerv( GL_BLEND_SRC_RGB, &srcRGB ) );

		GL_CHECK( glGetIntegerv( GL_BLEND_DST_ALPHA, &destAlpha ) );
		GL_CHECK( glGetIntegerv( GL_BLEND_DST_RGB, &destRGB ) );

		GL_CHECK( glBlendFunc( newSrc, newDst ) );
	}

	~pushBlend_t( void )
	{
		GL_CHECK( glBlendFuncSeparate( srcRGB, destRGB, srcAlpha, destAlpha ) );
	}
};
