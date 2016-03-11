#include "util.h"
#include "glutil.h"
#include "effect_shader.h"
#include "q3bsp.h"

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

	GStageSlot( textureIndex );

	const gTextureImage_t& texParams = GTextureImage( texHandle );
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

		if ( offset > -1 )
			program.LoadInt( "sampler0", offset );

		program.LoadVec4( "imageTransform", transform );
		program.LoadVec2( "imageScaleRatio", texParams.imageScaleRatio );
	}

	GUnstageSlot();
}

gTextureHandle_t GU_LoadShaderTextures( Q3BspMap& map, gSamplerHandle_t sampler )
{
	gImageParamList_t shaderTextures;
	S_LoadShaders( &map, sampler, shaderTextures );
	gTextureMakeParams_t makeParams( shaderTextures, sampler );
	return GMakeTexture( makeParams );
}

void GU_ImmBegin( GLenum mode, const glm::mat4& view, const glm::mat4& proj )
{
	GL_CHECK( glUseProgram( 0 ) );
	GL_CHECK( glMatrixMode( GL_PROJECTION ) );
	GL_CHECK( glLoadMatrixf( glm::value_ptr( proj ) ) );

	GL_CHECK( glMatrixMode( GL_MODELVIEW ) );
	GL_CHECK( glPushMatrix() );
	GL_CHECK( glLoadMatrixf( glm::value_ptr( view ) ) );

	glBegin( mode );
}

void GU_ImmLoad( const guImmPosList_t& v, const glm::vec4& color )
{
	for ( const auto& p: v )
	{
		glColor4fv( glm::value_ptr( color ) );
		glVertex3fv( glm::value_ptr( p ) );
	}
}

void GU_ImmEnd( void )
{
	glEnd();
	GL_CHECK( glMatrixMode( GL_MODELVIEW ) );
	GL_CHECK( glPopMatrix() );
}

void GU_ImmDrawLine( const glm::vec3& origin,
					 const glm::vec3& dir,
					 const glm::vec4& color,
					 const glm::mat4& view,
					 const glm::mat4& proj )
{
	GU_ImmBegin( GL_LINES, view, proj );
	glColor4fv( glm::value_ptr( color ) );
	glVertex3fv( glm::value_ptr( origin ) );
	glVertex3fv( glm::value_ptr( dir ) );
	GU_ImmEnd();
}

