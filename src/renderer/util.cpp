#include "util.h"
#include "glutil.h"

void GU_SetupTexParams( const Program& program,
                        const char* uniformPrefix,
                        gTextureHandle_t texHandle,
                        int32_t textureIndex,
                        uint32_t sampler,
                        uint32_t offset )
{
    if ( textureIndex < 0 )
    {
        if ( uniformPrefix )
            program.LoadInt( std::string( uniformPrefix ) + ".active", 0 );
        GReleaseTexture( texHandle, offset );
        return;
    }

    const gTextureImage_t& texParams = GTextureImage( texHandle, textureIndex );
    glm::vec2 invRowPitch( GTextureInverseRowPitch( texHandle ) );

    glm::vec4 transform;
    transform.x = texParams.stOffsetStart.s;
    transform.y = texParams.stOffsetStart.t;
    transform.z = invRowPitch.x;
    transform.w = invRowPitch.y;

    GBindTexture( texHandle );
    GL_CHECK( glBindSampler( offset, sampler ) );

    // If true, we're using the main program
    if ( uniformPrefix )
    {
        std::string prefix( uniformPrefix );

        program.LoadInt( prefix + ".active", 1 );
        program.LoadInt( prefix + ".sampler0", offset );
        program.LoadVec4( prefix + ".imageTransform", transform );
        program.LoadVec2( prefix + ".imageScaleRatio", texParams.imageScaleRatio );
    }
    else // otherwise, we have an effect shader
    {
        program.LoadInt( "sampler0", offset );
        program.LoadVec4( "imageTransform", transform );
        program.LoadVec2( "imageScaleRatio", texParams.imageScaleRatio );
    }
}
