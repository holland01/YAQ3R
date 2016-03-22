#pragma once

#include "common.h"

struct gImageParams_t;

struct atlasPositionMap_t
{
	const gImageParams_t* image = nullptr;
	glm::vec2 origin;
};

std::vector< atlasPositionMap_t > AtlasGenOrigins( const std::vector< gImageParams_t >& params,
												uint16_t maxTextureSize );

