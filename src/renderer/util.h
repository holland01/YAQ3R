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

// This function is meant to handle shared resources; GLES 2 doesn't really have much
// of a mechanism for that AFAIK, so we'll be emulating that behavior here, eventually.
static INLINE void GU_LoadTransforms( const glm::mat4& view, const glm::mat4& projection )
{
#ifdef GLES
    UNUSED( view );
    UNUSED( projection );
    MLOG_ERROR( "This function needs a GLES compatible implementation" );
#else
    GL_CHECK( glBindBuffer( GL_UNIFORM_BUFFER, transformBlockObj ) );
    GL_CHECK( glBufferSubData( GL_UNIFORM_BUFFER, 0, sizeof( glm::mat4 ), glm::value_ptr( projection ) ) );
    GL_CHECK( glBufferSubData( GL_UNIFORM_BUFFER, sizeof( glm::mat4 ), sizeof( glm::mat4 ), glm::value_ptr( view ) ) );
    GL_CHECK( glBindBuffer( GL_UNIFORM_BUFFER, 0 ) );
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
