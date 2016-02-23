#pragma once

#include "common.h"
#include "renderer_local.h"

struct Program;

struct gProgramHandle_t
{
	uint32_t id;
};

gProgramHandle_t GStoreShaderProgram( Program* p );

