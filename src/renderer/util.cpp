#include "util.h"
#include "glutil.h"
#include "effect_shader.h"
#include "shader_gen.h"
#include "lib/async_image_io.h"
#include "renderer.h"
#include "em_api.h"
#include "extern/gl_atlas.h"
#include <iostream>

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

static void PrintInfo( const std::string& title,
	const std::vector< gPathMap_t > source )
{
#ifdef DEBUG
	MLOG_INFO( "%s\n\tCount: %i", title.c_str(), ( int ) source.size() );
#else
	UNUSED( PrintInfo );
	UNUSED( title );
	UNUSED( source );
#endif
}

static gImnAutoPtr_t BundleImagePaths( std::vector< gPathMap_t >& sources )
{
	std::vector< gPathMap_t > env, gfx, models,
		sprites, textures;

	// Iterate over our path sources
	// and find each source a corresponding bundle
	for ( gPathMap_t& source: sources )
	{
		size_t slashPos = source.path.find_first_of( "/" );

		if ( slashPos == std::string::npos )
		{
			MLOG_INFO(
				"Invalid image path received: path \'%s\'"
				" does not belong to a bundle. Skipping",
				source.path.c_str() );
			continue;
		}

		AIIO_FixupAssetPath( source );

		// skip asset prefix
		const char* arg = strstr( &source.path[ 1 ], "/" ) + 1;

		switch ( BspData_GetAssetBaseFromPath( arg, nullptr ) )
		{
			case BSP_ASSET_BASE_ENV: env.push_back( source ); break;
			case BSP_ASSET_BASE_GFX: gfx.push_back( source ); break;
			case BSP_ASSET_BASE_MODELS: models.push_back( source ); break;
			case BSP_ASSET_BASE_SPRITES: sprites.push_back( source ); break;
			case BSP_ASSET_BASE_TEXTURES: textures.push_back( source ); break;

			default: 
				MLOG_ERROR( "%s", "Invalid bundle module received" );
				break;
		}

/*
		// Grab the path segment in between
		// the first two slashes: this is our
		// bundle we wish to assign
		// the current path source.
		const char* path = &source.path[ 1 ]; // We skip, since the first char is a slash.
		const char* slash0 = strstr( path, "/" );
		const char* slash1 = strstr( slash0 + 1, "/" );

		size_t len = slash1 - slash0 - 1;

		path = slash0 + 1;

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
		else
		{
			MLOG_ERROR( "%s", "Invalid bundle module received" );
		}
*/
	}

	gImageMountNode_t* h = new gImageMountNode_t();
	gImageMountNode_t** pn = &h;

	auto LAddNode = [ &pn ](
		const std::vector< gPathMap_t >& paths,
		const char* name
	)
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

	gla::atlas_t* destAtlas = nullptr;

	bool keyMapped = false;
};

static gLoadImagesState_t gImageLoadState;

static void LoadImagesBegin( char* mem, int size, void* param );

// NOTE:
// Each time a new bundle is loaded, the global image tracker
// in lib/async_image_io is reallocated. Since LoadImagesEnd()
// is passed to AIIO_ReadImages() (via LoadImages()) as a
// callback - which is invoked once all of desired the images from that particular bundle
// have been read - then it's up to the finishing callback to actually delete
// the memory owned by the current global image tracker.

// This _isn't_ a good design - it was naiively written this way a year ago.
// It should be rewritten.
// That said, given the current scope of the application,
// the amount of time available for yours truly, and the fact that I want
// to actually finish this project, it's going to have to remain that way :/

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

static void LoadImagesBegin( char* mem, int size, void* param )
{
	UNUSED( mem );
	UNUSED( size );
	UNUSED( param );

	if ( gImageLoadState.currNode )
	{
		AIIO_ReadImages(
			gImageLoadState.map,
			gImageLoadState.currNode->bundle,
			gImageLoadState.currNode->paths,
			LoadImagesEnd,
			gImageLoadState.destAtlas,
			gImageLoadState.keyMapped
		);
	}
	else
	{
		gImageLoadState.head.reset();
		gImageLoadState.destAtlas = nullptr;
		gImageLoadState.mapLoadFinEvent( param );
	}
}

static void LoadImageState(
	Q3BspMap& map,
	std::vector< gPathMap_t >& sources,
	int atlas
)
{
	gImageLoadState.map = &map;
	gImageLoadState.head = BundleImagePaths( sources );
	gImageLoadState.currNode = gImageLoadState.head.get();
	gImageLoadState.destAtlas = map.payload->textureData[ atlas ].get();

	LoadImagesBegin( nullptr, 0, 0 );
}

void GU_LoadShaderTextures( Q3BspMap& map )
{
	for ( auto& entry: map.effectShaders )
	{
		GMakeProgramsFromEffectShader( entry.second );
	}

	std::vector< gPathMap_t > sources = map.GetShaderSourcesList();

	gImageLoadState.keyMapped = false;
	gImageLoadState.mapLoadFinEvent = Q3BspMap::OnShaderLoadImagesFinish;

	LoadImageState( map, sources, TEXTURE_ATLAS_SHADERS );
}

void GU_LoadMainTextures( Q3BspMap& map )
{
	std::vector< gPathMap_t> sources;

	for ( size_t key = 0; key < map.data.shaders.size(); ++key )
	{
		gPathMap_t initial;

		initial.path = std::string( map.data.shaders[ key ].name );

		bool needed = true;

		for ( auto& entry: map.effectShaders )
		{
			if ( entry.first == initial.path )
			{
				needed = false;
				break;
			}
		}

		if ( needed )
		{
			Q3BspMapTest_ShaderNameTagMain(
				&map.data.shaders[ key ].name[ 0 ] );

			initial.param = ( void* ) key;
			sources.push_back( initial );
		}
	}

	gImageLoadState.keyMapped = true;
	gImageLoadState.mapLoadFinEvent = Q3BspMap::OnMainLoadImagesFinish;

	LoadImageState( map, sources, TEXTURE_ATLAS_MAIN );
}

void GU_LoadDebugTextures( Q3BspMap& map )
{
	std::vector< gPathMap_t > sources;

	for ( size_t i = 0; i < map.debugTexturePaths.size(); ++i )
	{
		gPathMap_t initial {
			map.debugTexturePaths[ i ],
			( void* ) i
		};

		sources.push_back( initial );
	}

	gImageLoadState.keyMapped = true;
	gImageLoadState.mapLoadFinEvent = Q3BspMap::OnDebugLoadImagesFinish;

	LoadImageState( map, sources, TEXTURE_ATLAS_DEBUG );
}

struct diffZCache_t
{
	float zMin = -1.0f;
	float zMax = -1.0f;
	float iDiffZ = 0.0f;
};

static diffZCache_t gDiffZCache; // division is slow so we cache the inverse

size_t GU_MapViewDepthToInt( float viewZDepth, float zMin, float zMax )
{
	float zDiff = zMax - zMin; 

	// prevent divide by zero
	if ( zDiff < 0.1f && zDiff >= 0.0f )
	{
		return G_MAPPED_DEPTH_CLIPPED;
	}

	// view space looks down -Z
	float zDepth = -viewZDepth; 

	if ( zDepth < zMin || zDepth > zMax )
	{
		return G_MAPPED_DEPTH_CLIPPED;
	}

	// update if we need to
	if ( zMin != gDiffZCache.zMin || zMax != gDiffZCache.zMax )
	{
		gDiffZCache.zMax = zMax;
		gDiffZCache.zMin = zMin;
		gDiffZCache.iDiffZ = 1.0f / zDiff;
	}

	float normalized = zMax * gDiffZCache.iDiffZ - zMax * zMin * gDiffZCache.iDiffZ / zDiff;

	size_t ret = ( size_t )( normalized * ( float ) DRAWFACE_SORT_DEPTH_MASK );
	return ret;
}
