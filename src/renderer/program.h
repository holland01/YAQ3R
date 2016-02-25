#pragma once

#include "common.h"
#include "renderer_local.h"

class Program;

struct gProgramHandle_t
{
	uint32_t id;
};

gProgramHandle_t GStoreShaderProgram( Program* p );

