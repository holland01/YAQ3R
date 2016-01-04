#include "util.h"
#include "glutil.h"

void GU_SetupTexParams( const Program& program,
                        const char* uniformPrefix,
                        gTextureHandle_t texHandle,
                        int32_t textureIndex,
                        uint32_t offset )
{
    if ( textureIndex < 0 )
    {
        textureIndex = 0;
        if ( uniformPrefix )
            program.LoadInt( std::string( uniformPrefix ) + "Active", 0 );
        GReleaseTexture( texHandle, offset );
        return;
    }

    const gTextureImage_t& texParams = GTextureImage( texHandle, textureIndex );
    glm::vec2 invRowPitch( GTextureInverseRowPitch( texHandle ) );

    glm::vec4 transform;
    transform.x = texParams.stOffsetStart.x;
    transform.y = texParams.stOffsetStart.y;
    transform.z = invRowPitch.x;
    transform.w = invRowPitch.y;

    GBindTexture( texHandle );

    // If true, we're using the main program
    if ( uniformPrefix )
    {
        std::string prefix( uniformPrefix );

        program.LoadInt( prefix + "Sampler", offset );
        program.LoadInt( prefix + "Active", 1 );
        program.LoadVec4( prefix + "ImageTransform", transform );
        program.LoadVec2( prefix + "ImageScaleRatio", texParams.imageScaleRatio );
    }
    else // otherwise, we have an effect shader
    {
        program.LoadInt( "sampler0", offset );
        program.LoadVec4( "imageTransform", transform );
        program.LoadVec2( "imageScaleRatio", texParams.imageScaleRatio );
    }
}
