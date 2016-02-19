#include "util.h"
#include "glutil.h"

void GU_SetupTexParams( const Program& program,
						const char* uniformPrefix,
						gTextureHandle_t texHandle,
						int32_t textureIndex,
						int32_t offset )
{
	if ( textureIndex < 0 )
	{
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

	GBindTexture( texHandle, offset );

	// If true, we're using the main program
	if ( uniformPrefix )
	{
		std::string prefix( uniformPrefix );

		if ( offset > -1 )
			program.LoadInt( prefix + "Sampler", offset );

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
