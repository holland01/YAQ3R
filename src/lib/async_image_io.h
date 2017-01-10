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

// Texture images are sampled from large atlasses. The idea is that
// an index is used to obtain the necessary parameters (from the texture system)
// to properly sample the image in the fragment shader.

// As a result, we have to keep track of the indices for each image we generate,
// either via a shaderStage_t or directly. For non-shader-based images, each
// index has to correspond to a specific bspShader_t record from the
// map data buffer.

// In this case, keyMaps are used given that some of
// the bspShader_t name entries won't
// represent a specific image.

// For texture images directly belonging to a shaderStage_t (which has been
// generated from the effect shader loader), we have to make sure that
// every stage with a texture image knows the index used to access the
// "image slot" within the texture atlas.

struct gImageLoadTracker_t
{
	bool isKeyMapped : 1;
	int16_t iterator;
	int16_t extIterator;

	onFinishEvent_t finishEvent;

	gImageParamList_t textures;

	glm::ivec2 maxDims;

	Q3BspMap& map;

	std::vector< gPathMap_t > textureInfo;

	gImageLoadTracker_t( Q3BspMap& map_,
		std::vector< gPathMap_t > textureInfo_,
		gSamplerHandle_t sampler_,
		onFinishEvent_t finishEvent_,
		bool isKeyMapped_ )
		:	isKeyMapped( isKeyMapped_ ),
			iterator( 0 ),
			extIterator( 0 ),
			finishEvent( finishEvent_ ),
			sampler( sampler_ ),
			maxDims( 0.0f ),
			map( map_ ),
			textureInfo( textureInfo_ )
	{
	}

	void LogImages( void );
};

void AIIO_FixupAssetPath( gPathMap_t& pathMap );

void AIIO_ReadImages(
	Q3BspMap& map,
	std::vector< gPathMap_t > pathInfo,
	onFinishEvent_t finish,
	bool keyMapped );
