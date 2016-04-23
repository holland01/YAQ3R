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

struct gImageLoadTracker_t
{
	int32_t iterator;

	onFinishEvent_t finishEvent;
	onFinishEvent_t insertEvent;

	gSamplerHandle_t sampler;
	gImageParamList_t textures;

	glm::ivec2 maxDims;

	Q3BspMap& map;
	std::vector< gPathMap_t > textureInfo;

	gImageLoadTracker_t( Q3BspMap& map_, std::vector< gPathMap_t > textureInfo_ )
		:	iterator( 0 ),
			finishEvent( nullptr ),
			insertEvent( nullptr ),
			maxDims( 0.0f ),
			map( map_ ),
			textureInfo( textureInfo_ )
	{
	}
};

extern std::unique_ptr< gImageLoadTracker_t > gImageTracker;

void AIIO_ReadImages( Q3BspMap& map, std::vector< gPathMap_t > pathInfo,
	gSamplerHandle_t sampler, onFinishEvent_t finish, onFinishEvent_t insert );
