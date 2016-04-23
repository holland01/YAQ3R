#include "util.h"
#include "glutil.h"
#include "effect_shader.h"
#include "shader_gen.h"
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

	glm::ivec2 maxDims( 0 );
	for ( auto& entry: map.effectShaders )
	{
		GMakeProgramsFromEffectShader( entry.second );
	}

	for ( auto& entry: map.effectShaders )
	{
		for ( int i = 0; i < entry.second.stageCount; ++i )
		{
			GU_LoadStageTexture( maxDims, shaderTextures, entry.second, i, sampler );
		}
	}

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

void GU_LoadStageTexture( glm::ivec2& maxDims, std::vector< gImageParams_t >& images,
	shaderInfo_t& info, int i, const gSamplerHandle_t& sampler )
{
	shaderStage_t& stage = info.stageBuffer[ i ];

	if ( stage.mapType == MAP_TYPE_IMAGE )
	{
		gImageParams_t img;
		img.sampler = sampler;

		// If a texture atlas is being used as a substitute for a texture array,
		// this won't matter.
		std::string texFileRoot( ASSET_Q3_ROOT );

		// texturePath is a char array, so it's simpler to just append
		// the slash to texFileRoot (ASSET_Q3_ROOT has no trailing slash by design)
		if ( stage.texturePath[ 0 ] != '/' )
		{
			texFileRoot.append( 1, '/' );
		}

		std::string texRelativePath( &stage.texturePath[ 0 ],
			strlen( &stage.texturePath[ 0 ] ) );
		texFileRoot.append( texRelativePath );

		// If it's a tga file and we fail, then chances are there is a jpeg duplicate
		// of it that we can fall back on
		if ( !GLoadImageFromFile( texFileRoot.c_str(), img ) )
		{
			std::string ext;
			size_t index;
			if ( File_GetExt( ext, &index, texFileRoot ) && ext == "tga" )
			{
				texFileRoot.replace( index, 4, ".jpg" );
				if ( !GLoadImageFromFile( texFileRoot, img ) )
				{
					// If we fail second try, just leave it: it will be rendered as a dummy
					MLOG_WARNING( "TGA image asset request. Not found; tried jpeg as an alternative - no luck. File \"%s\"", texFileRoot.c_str() );
					return;
				}
			}
		}

		// We need the highest dimensions out of all images for the texture array
		maxDims.x = glm::max( img.width, maxDims.x );
		maxDims.y = glm::max( img.height, maxDims.y );

		// This index will persist in the texture array it's going into
		stage.textureIndex = images.size();

		images.push_back( img );
	}
}

#ifndef EMSCRIPTEN
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
#endif // EMSCRIPTEN
