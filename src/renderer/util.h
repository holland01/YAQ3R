#pragma once

#include "common.h"
#include "texture.h"
#include "glutil.h"

void GU_SetupTexParams( const Program& program,
                        const char* uniformPrefix,
                        gTextureHandle_t texHandle,
                        int32_t textureIndex,
                        uint32_t offset = 0 );


static INLINE void GU_ClearDepth( float d )
{
#ifdef GLES
    GL_CHECK( glClearDepthf( d ) );
#else
    GL_CHECK( glClearDepth( d ) );
#endif
}

using indexBufferList_t = std::vector< const int32_t* >;
using indexBufferSizeList_t = std::vector< int32_t >;

static INLINE void GU_MultiDrawElements( GLenum mode, const indexBufferList_t& indexBuffers,
                                         const indexBufferSizeList_t& indexBufferSizes )
{
    for ( uint32_t i = 0; i < indexBuffers.size(); ++i )
        GL_CHECK( glDrawElements( mode, indexBufferSizes[ i ], GL_UNSIGNED_INT, indexBuffers[ i ] ) );
}
