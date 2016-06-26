#pragma once

#include "common.h"
#include "renderer/texture.h"
#include <memory>

class Q3BspMap;

struct gPathMap_t
{
	std::string path;
	void* param = nullptr;
};

using extFallbackBuff_t = std::vector< std::string >;

struct gImageLoadTracker_t
{
	int16_t iterator;
	int16_t extIterator;

	onFinishEvent_t finishEvent;
	onFinishEvent_t insertEvent;

	gSamplerHandle_t sampler;
	gImageParamList_t textures;

	glm::ivec2 maxDims;

	Q3BspMap& map;
	std::vector< gTextureImageKey_t > indices; // in the event
	// we wish to index textures by key, as opposed to a one->one lookup
	// dictated by the amount of paths specified.
	std::vector< gPathMap_t > textureInfo;
	extFallbackBuff_t fallbackExts; // used for when an image fails
	// to load due to path error; there's a chance that the same image exists,
	// just with a different extension (e.g., fail on .tga, but there is a .jpeg)

	gImageLoadTracker_t( Q3BspMap& map_, std::vector< gPathMap_t > textureInfo_,
	 	const extFallbackBuff_t& fallbackExts_ )
		:	iterator( 0 ),
			extIterator( 0 ),
			finishEvent( nullptr ),
			insertEvent( nullptr ),
			maxDims( 0.0f ),
			map( map_ ),
			textureInfo( textureInfo_ ),
			fallbackExts( fallbackExts_ )
	{
	}

	bool FallbackEnd( void ) const { return extIterator == ( int16_t ) fallbackExts.size(); }
	void ResetFallback( void ) { extIterator = 0; }
	bool NextFallback( void );

	void LogImages( void );
};

extern std::unique_ptr< gImageLoadTracker_t > gImageTracker;

gPathMap_t AIIO_MakeAssetPath( const char* path );

void AIIO_ReadImages( Q3BspMap& map, std::vector< gPathMap_t > pathInfo,
	std::vector< std::string > fallbackExts, gSamplerHandle_t sampler,
	onFinishEvent_t finish, onFinishEvent_t insert );
