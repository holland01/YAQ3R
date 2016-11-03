#pragma once

#include "common.h"

struct gImageParams_t;

// NOTE: origins[ i ] corresponds to params[ i ],
// where params is the vector< gImageParams_t > received
// in AtlasGenOrigins()
struct atlasBaseInfo_t
{
	std::vector< glm::vec2 > origins;
	uint16_t width;
	uint16_t height;
};

atlasBaseInfo_t AtlasGenOrigins( const std::vector< gImageParams_t >& params,
												uint16_t maxTextureSize );
