#pragma once

#include "common.h"
#include "io.h"
#include <memory>
#include <unordered_map>

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

namespace gla
{
	struct atlas_t;
}

struct gImageLoadTracker_t
{
	bool isKeyMapped : 1;

	onFinishEvent_t finishEvent;

	Q3BspMap* map;

	std::unordered_map< std::string, void* > textureInfo;

	size_t serverImageCount;
	size_t iterator;

	gla::atlas_t* destAtlas;

	std::unordered_map< std::string, void* >
		GenInfoMap( const std::vector< gPathMap_t >& info ) const
	{
		std::unordered_map< std::string, void* > infoMap;

		for ( auto& entry: info )
		{
			// Server will send image filenames without extensions:
			// there are a fair amount of paths referenced in the shader
			// files which exist on disk, but in different formats
			// than what is specified in their corresponding shader entries.
			infoMap[ File_StripExt( entry.path ) ] = entry.param;
		}

		return infoMap;
	}

	gImageLoadTracker_t(
		Q3BspMap* map_,
		const std::vector< gPathMap_t >& textureInfo_,
		onFinishEvent_t finishEvent_,
		gla::atlas_t* destAtlas_,
		bool isKeyMapped_
	)	:	isKeyMapped( isKeyMapped_ ),
			finishEvent( finishEvent_ ),
			map( map_ ),
			textureInfo( GenInfoMap( textureInfo_ ) ),
			serverImageCount( 0 ),
			iterator( 0 ),
			destAtlas( destAtlas_ )
	{
	}

	void LogImages( void );
};

using gImageLoadTrackerPtr_t = std::unique_ptr< gImageLoadTracker_t >;

void AIIO_FixupAssetPath( gPathMap_t& pathMap );

void AIIO_ReadImages(
	Q3BspMap* map,
	const std::string& bundlePath,
	const std::vector< gPathMap_t >& pathInfo,
	onFinishEvent_t finish,
	gla::atlas_t* destAtlas,
	bool keyMapped
);
