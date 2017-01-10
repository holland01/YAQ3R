#pragma once

#include "glutil.h"

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

static INLINE void GU_DrawElements(
	GLenum mode,
	guOffset_t buffOffset,
	GLsizei numIndices
)
{
	GL_CHECK(
		glDrawElements(
			mode,
			numIndices,
			GL_UNSIGNED_INT,
			( const GLvoid* )( buffOffset * G_INDEX_BYTE_STRIDE ) ) );
}

static INLINE void GU_MultiDrawElements(
	GLenum mode,
	const guBufferOffsetList_t& indexBuffers,
	const guBufferRangeList_t& indexBufferSizes
)
{
	for ( uint32_t i = 0; i < indexBuffers.size(); ++i )
	{
		GU_DrawElements( mode, indexBuffers[ i ], indexBufferSizes[ i ] );
	}
}

class Q3BspMap;

void GU_LoadShaderTextures( Q3BspMap& map );

void GU_LoadMainTextures( Q3BspMap& map );

using guImmPosList_t = std::vector< glm::vec3 >;

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
