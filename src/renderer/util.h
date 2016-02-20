#pragma once

#include "common.h"
#include "texture.h"
#include "glutil.h"

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

static INLINE void GU_MultiDrawElements( GLenum mode, const guBufferOffsetList_t& indexBuffers, const guBufferRangeList_t& indexBufferSizes )
{
	for ( uint32_t i = 0; i < indexBuffers.size(); ++i )
		GL_CHECK( glDrawElements( mode, indexBufferSizes[ i ], GL_UNSIGNED_INT, ( const GLvoid* )( indexBuffers[ i ] * 4 ) ) );
}
