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

gTextureHandle_t GU_LoadMainTextures( Q3BspMap& map, gSamplerHandle_t sampler )
{
	//---------------------------------------------------------------------
	// Load Textures:
	// This is just a hack to brute force load assets which don't belong in shaders.
	// Now, we find and generate the textures. We first start with the image files.
	//---------------------------------------------------------------------

	const char* validImgExt[] =
	{
		".jpg", ".png", ".tga", ".tiff", ".bmp"
	};

	gImageParamList_t textures;
	std::vector< gTextureImageKey_t > indices;

	for ( int32_t t = 0; t < map.data.numShaders; t++ )
	{
		// We pre-initialize these before needing them because of the goto.
		std::string fname( map.data.shaders[ t ].name );
		const std::string& texPath = map.data.basePath + fname;

		gImageParams_t texture;
		texture.sampler = sampler;

		bool success = false;

		// No use in allocating tex memory if this is meant to be used with a shader
		if ( map.GetShaderInfo( map.data.shaders[ t ].name ) )
		{
			MLOG_INFO( "Shader found for: \'%s\'; skipping.", map.data.shaders[ t ].name );
			continue;
		}

		// If we don't have a file extension appended in the name,
		// try to find one for it which is valid
		{
			for ( int32_t i = 0; i < SIGNED_LEN( validImgExt ); ++i )
			{
				const std::string& str = texPath + std::string( validImgExt[ i ] );

				if ( GLoadImageFromFile( str, texture ) )
				{
					success = true;
					indices.push_back( t );
					textures.push_back( texture );
					break;
				}
			}
		}

		if ( !success )
		{
			MLOG_WARNING( "Could not find a file extension for \'%s\'", texPath.c_str() );
		}
	}

	{
		// We want to maintain a one->one mapping with the texture indices in the bsp file,
		// so we ensure the indices are properly mapped
		gTextureMakeParams_t makeParams( textures, sampler, G_TEXTURE_STORAGE_KEY_MAPPED_BIT );
		makeParams.keyMaps = std::move( indices );
		return GMakeTexture( makeParams );
	}
}

void GU_ImmLoadMatrices( const glm::mat4& view, const glm::mat4& proj )
{
	GL_CHECK( glUseProgram( 0 ) );
	GL_CHECK( glMatrixMode( GL_PROJECTION ) );
	GL_CHECK( glLoadMatrixf( glm::value_ptr( proj ) ) );

	GL_CHECK( glMatrixMode( GL_MODELVIEW ) );
	GL_CHECK( glPushMatrix() );
	GL_CHECK( glLoadMatrixf( glm::value_ptr( view ) ) );
}

void GU_ImmBegin( GLenum mode, const glm::mat4& view, const glm::mat4& proj )
{
	GU_ImmLoadMatrices( view, proj );
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

void GU_ImmLoad( const guImmPosList_t& v, const glm::u8vec4& color )
{
	for ( const auto& p: v )
	{
		glColor4ubv( glm::value_ptr( color ) );
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

