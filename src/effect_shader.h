#pragma once

#include "common.h"
#include "glutil.h"
#include <memory>

struct gImageParams_t;
struct gSamplerHandle_t;

class Q3BspMap;

glm::ivec2 S_LoadShaders( Q3BspMap* map, const gSamplerHandle_t& imageSampler, std::vector< gImageParams_t >& textures );

void S_GenPrograms( shaderInfo_t& shader );

std::string S_GetGLSLHeader( void );

std::string S_MainVertexShader( void );

std::string S_MainFragmentShader( void );

bool operator == ( const std::array< char, SHADER_MAX_TOKEN_CHAR_LENGTH >& str1, const char* str2 );

static INLINE bool S_StageHasIdentityColor( const shaderStage_t& s )
{
	return s.rgbGen == RGBGEN_IDENTITY || s.rgbGen == RGBGEN_IDENTITY_LIGHTING;
}
