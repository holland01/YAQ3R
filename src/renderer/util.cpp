#include "util.h"
#include "glutil.h"
#include "effect_shader.h"
#include "shader_gen.h"
#include "lib/async_image_io.h"
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
		program.LoadVec2( prefix + "ImageScaleRatio", 
			texParams.imageScaleRatio );
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

static void PreInsert_Shader( void* param )
{
	gImageLoadTracker_t* imageTracker = ( gImageLoadTracker_t* )param;
	{
		shaderStage_t* stage =
			( shaderStage_t* )imageTracker->
				textureInfo[ imageTracker->iterator ].param;

		// This index will persist in the texture array it's going into
		stage->textureIndex = imageTracker->textures.size();
	}
	/*
	// We need the highest dimensions out of all images for the texture array
	{
		const gImageParams_t* image = ( gImageParams_t* )param;

		gImageTracker->maxDims.x = glm::max( image->width,
			gImageTracker->maxDims.x );

		gImageTracker->maxDims.y = glm::max( image->height,
			gImageTracker->maxDims.y );
	}
	*/
}

void GU_LoadShaderTextures( Q3BspMap& map,
	gSamplerHandle_t sampler )
{
	std::vector< gPathMap_t > paths;

	for ( auto& entry: map.effectShaders )
	{
		GMakeProgramsFromEffectShader( entry.second );
	}

	for ( auto& entry: map.effectShaders )
	{
		for ( shaderStage_t& stage: entry.second.stageBuffer )
		{
			if ( stage.mapType == MAP_TYPE_IMAGE )
			{
				gPathMap_t pathMap = AIIO_MakeAssetPath( 
					&stage.texturePath[ 0 ] );
				MLOG_INFO( "pathMap.path: %s\n", pathMap.path.c_str() );
				pathMap.param = &stage;
				paths.push_back( pathMap );
			}
		}
	}

	AIIO_ReadImages( map, paths, sampler, Q3BspMap::OnShaderLoadTexturesFinish,
		PreInsert_Shader );
}

static void PreInsert_Main( void* param )
{
	gImageLoadTracker_t* imageTracker = ( gImageLoadTracker_t* )param;
	imageTracker->indices.push_back( imageTracker->iterator );
}

void GU_LoadMainTextures( Q3BspMap& map, gSamplerHandle_t sampler )
{
	std::vector< gPathMap_t > paths;
	paths.reserve( map.data.shaders.size() );

	int i = 0;
	for ( auto& s: map.data.shaders )
	{
		paths.push_back( AIIO_MakeAssetPath( s.name ) );
		i++;
	}

	AIIO_ReadImages( map, paths,  sampler, Q3BspMap::OnMainLoadTexturesFinish,
		PreInsert_Main );

	//---------------------------------------------------------------------
	// Load Textures:
	// This is just a hack to brute force load assets which don't belong in 
	//	shaders.
	// Now, we find and generate the textures. We first start with the 
	// image files.
	//---------------------------------------------------------------------
/*
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

		// No use in allocating tex memory if this is meant to be 
		// used with a shader
		if ( map.GetShaderInfo( map.data.shaders[ t ].name ) )
		{
			MLOG_INFO( "Shader found for: \'%s\'; skipping.", 
				map.data.shaders[ t ].name );
			continue;
		}

		// If we don't have a file extension appended in the name,
		// try to find one for it which is valid
		{
			for ( int32_t i = 0; i < SIGNED_LEN( validImgExt ); ++i )
			{
				const std::string& str = texPath + 
					std::string( validImgExt[ i ] );

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
		// We want to maintain a one->one mapping with the texture 
		// indices in the bsp file,
		// so we ensure the indices are properly mapped
		gTextureMakeParams_t makeParams( textures, sampler, 
			G_TEXTURE_STORAGE_KEY_MAPPED_BIT );
		makeParams.keyMaps = std::move( indices );
		return GMakeTexture( makeParams );
	}
	*/
}

void GU_LoadStageTexture( glm::ivec2& maxDims, 
		std::vector< gImageParams_t >& images,
		shaderInfo_t& info, int i, const gSamplerHandle_t& sampler )
{
	UNUSED(maxDims);
	UNUSED(images);
	UNUSED(info);
	UNUSED(i);
	UNUSED(sampler);
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
