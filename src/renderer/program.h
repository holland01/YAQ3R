#pragma once

#include "common.h"
#include "renderer_local.h"

struct gProgramHandle_t
{
	uint32_t id;
};

struct shaderStage_t;

gProgramHandle_t GFindProgramByData( const programDataMap_t& attribs, const programDataMap_t& uniforms, const shaderStage_t* stage );

class Program;

gProgramHandle_t GStoreProgram( Program* p );

Program* GQueryProgram( gProgramHandle_t handle );

const Program& GQueryProgram( gProgramHandle_t handle );
