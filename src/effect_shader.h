#pragma once

#include "common.h"
#include "glutil.h"
#include <memory>


struct gImageParams_t;

glm::ivec2 S_LoadShaders( const mapData_t* map, std::vector< gImageParams_t >& textures, shaderMap_t& effectShaders );

bool operator == ( const std::array< char, SHADER_MAX_TOKEN_CHAR_LENGTH >& str1, const char* str2 );

static INLINE bool S_StageHasIdentityColor( const shaderStage_t& s )
{
	return s.rgbGen == RGBGEN_IDENTITY || s.rgbGen == RGBGEN_IDENTITY_LIGHTING;
}
