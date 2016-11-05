#include "util.h"
#include "glutil.h"
#include "effect_shader.h"
#include "shader_gen.h"
#include "lib/async_image_io.h"
#include "q3bsp.h"
#include "em_api.h"

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

	gTextureImageShaderParams_t shaderParams =
		GTextureImageShaderParams( texHandle, textureIndex );

	GBindTexture( texHandle, offset );

	// If true, we're using the main program
	if ( uniformPrefix )
	{
		std::string prefix( uniformPrefix );

		if ( offset > -1 )
			program.LoadInt( prefix + "Sampler", offset );

		program.LoadVec4( prefix + "ImageTransform",
			shaderParams.transform );
		program.LoadVec2( prefix + "ImageScaleRatio",
			shaderParams.dimensions );
	}
	else // otherwise, we have an effect shader
	{
		if ( offset > -1 )
			program.LoadInt( "sampler0", offset );

		program.LoadVec4( "imageTransform", shaderParams.transform );
		program.LoadVec2( "imageScaleRatio", shaderParams.dimensions );
	}
}

struct gImageMountNode_t
{
	std::vector< gPathMap_t > paths;
	std::string bundle;
	gImageMountNode_t* next = nullptr;
};

using gImnAutoPtr_t = std::unique_ptr< gImageMountNode_t,
	  std::function< void( gImageMountNode_t* ) > >;

void DestroyImageMountNodes( gImageMountNode_t* n )
{
	for ( const gImageMountNode_t* u = n; u; )
	{
		const gImageMountNode_t* v = u->next;
		delete u;
		u = v;
	}
}

static gImnAutoPtr_t BundleImagePaths( std::vector< gPathMap_t >& sources )
{
	std::vector< gPathMap_t > env, gfx, models,
		sprites, textures;

	for ( gPathMap_t& source: sources )
	{
		// Yes, this looks like total shit: we should be seeing only
		// std::string functions instead of this odd mix
		// of std::string with c string functions. There's no good reason
		// for this other than lots of rewriting and has been happening.
		// Time is currently of the essence, so it's hard to justify
		// rewriting it.

		const char* path = &source.path[ 0 ];
		const char* slash = strstr( path, "/" );

		if ( !slash )
		{
			MLOG_INFO(
				"Invalid image path received: path \'%s\'"
				" does not belong to a bundle. Skipping",
				path );
			continue;
		}

		size_t len = ( ptrdiff_t )( slash - path );

		AIIO_FixupAssetPath( source );

		if ( strncmp( path, "env", len ) == 0 )
		{
			env.push_back( source );
		}
		else if ( strncmp( path, "gfx", len ) == 0 )
		{
			gfx.push_back( source );
		}
		else if ( strncmp( path, "models", len ) == 0 )
		{
			models.push_back( source );
		}
		else if ( strncmp( path, "sprites", len ) == 0 )
		{
			sprites.push_back( source );
		}
		else if ( strncmp( path, "textures", len ) == 0 )
		{
			textures.push_back( source );
		}
	}

	gImageMountNode_t* h = new gImageMountNode_t();
	gImageMountNode_t** pn = &h;

	auto LAddNode = [ &pn ]( const std::vector< gPathMap_t >& paths,
			const char* name )
	{
		if ( !paths.empty() )
		{
			if ( !( *pn ) )
			{
				*pn = new gImageMountNode_t();
			}

			( *pn )->paths = std::move( paths );
			( *pn )->bundle = std::string( name );

			pn = &( ( *pn )->next );
		}
	};

	LAddNode( env, "env" );
	LAddNode( gfx, "gfx" );
	LAddNode( models, "models" );
	LAddNode( sprites, "sprites" );
	LAddNode( textures, "textures" );

	return gImnAutoPtr_t( h, DestroyImageMountNodes );
}

struct gLoadImagesState_t
{
	gImnAutoPtr_t head;
	gImageMountNode_t* currNode = nullptr;

	onFinishEvent_t mapLoadFinEvent = nullptr;
	Q3BspMap* map = nullptr;

	gSamplerHandle_t sampler = { G_UNSPECIFIED };

	bool keyMapped = false;
};

static gLoadImagesState_t gImageLoadState;

static void LoadImagesBegin( char* mem, int size, void* param );

static void LoadImagesEnd( void* param )
{
	gImageLoadState.currNode = gImageLoadState.currNode->next;
	gFileWebWorker.Await(
		LoadImagesBegin,
		"UnmountPackages",
		nullptr,
		0,
		param
	);
}

static void LoadImages( char* mem, int size, void* param )
{
	AIIO_ReadImages(
		*gImageLoadState.map,
		gImageLoadState.currNode->paths,
		gImageLoadState.sampler,
		LoadImagesEnd,
		gImageLoadState.keyMapped
	);
}

static void LoadImagesBegin( char* mem, int size, void* param )
{
	if ( gImageLoadState.currNode )
	{
		gFileWebWorker.Await(
			LoadImages,
			"MountPackage",
			gImageLoadState.currNode->bundle,
			nullptr
		);
	}
	else
	{
		gImageLoadState.head.reset();
		gImageLoadState.mapLoadFinEvent( param );
	}
}

static void LoadImageState( Q3BspMap& map, gSamplerHandle_t sampler,
	std::vector< gPathMap_t >& sources )
{
	gImageLoadState.map = &map;
	gImageLoadState.sampler = sampler;

	gImageLoadState.head = BundleImagePaths( sources );

	gImageLoadState.currNode = gImageLoadState.head.get();

	LoadImagesBegin( nullptr, 0, 0 );
}

void GU_LoadShaderTextures( Q3BspMap& map,
	gSamplerHandle_t sampler )
{
	for ( auto& entry: map.effectShaders )
	{
		GMakeProgramsFromEffectShader( entry.second );
	}

	std::vector< gPathMap_t > sources;
	for ( auto& entry: map.effectShaders )
	{
		for ( shaderStage_t& stage: entry.second.stageBuffer )
		{
			if ( stage.mapType == MAP_TYPE_IMAGE )
			{
				gPathMap_t initial;

				initial.param = &stage;
				initial.path = std::string( &stage.texturePath[ 0 ] );

				sources.push_back( initial );
			}
		}
	}

	gImageLoadState.keyMapped = false;
	gImageLoadState.mapLoadFinEvent = Q3BspMap::OnShaderLoadImagesFinish;

	LoadImageState( map, sampler, sources );
}

void GU_LoadMainTextures( Q3BspMap& map, gSamplerHandle_t sampler )
{
	std::vector< gPathMap_t> sources;

	for ( size_t key = 0; key < map.data.shaders.size(); ++key )
	{
		gPathMap_t initial;

		initial.path = std::string( map.data.shaders[ key ].name );
		initial.param = ( void* ) key;

		sources.push_back( initial );
	}

	gImageLoadState.keyMapped = true;
	gImageLoadState.mapLoadFinEvent = Q3BspMap::OnMainLoadImagesFinish;

	LoadImageState( map, sampler, sources );
}

void GU_LoadStageTexture( glm::ivec2& maxDims,
		std::vector< gImageParams_t >& images,
		shaderInfo_t& info,
		int i,
		const gSamplerHandle_t& sampler )
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
