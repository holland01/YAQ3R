#pragma once

#include "common.h"
#include "glutil.h"
#include <memory>

struct gImageParams_t;
struct gSamplerHandle_t;

class Q3BspMap;

void S_LoadShaders( Q3BspMap* map );

bool operator == ( const std::array< char, BSP_MAX_SHADER_TOKEN_LENGTH >& str1,
	const char* str2 );

static INLINE bool S_StageHasIdentityColor( const shaderStage_t& s )
{
	return s.rgbGen == RGBGEN_IDENTITY || s.rgbGen == RGBGEN_IDENTITY_LIGHTING;
}
