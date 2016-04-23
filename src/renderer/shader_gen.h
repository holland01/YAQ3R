#pragma once

#include "renderer_local.h"

struct shaderInfo_t;

void GMakeProgramsFromEffectShader( shaderInfo_t& shader );

std::string GGetGLSLHeader( void );

std::string GMakeMainVertexShader( void );

std::string GMakeMainFragmentShader( void );
