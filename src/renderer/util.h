#pragma once

#include "common.h"
#include "texture.h"
#include "glutil.h"

#define G_INDEX_BYTE_STRIDE 4

void GU_SetupTexParams( const Program& program,
						const char* uniformPrefix,
						gTextureHandle_t texHandle,
						int32_t textureIndex,
						int32_t offset );


static INLINE void GU_ClearDepth( float d )
{
#ifdef GLES
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
		GU_DrawElements( mode, indexBuffers[ i ], indexBufferSizes[ i ] );
}
