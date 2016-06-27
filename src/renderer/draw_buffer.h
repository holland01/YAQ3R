#pragma once

#include "renderer_local.h"
#include "common.h"

struct gDrawBufferHandle_t
{
	uint32_t id;
};

gDrawBufferHandle_t GAllocDrawBuffer( size_t elemSize, size_t numElements );

void GDrawDrawBuffer( gDrawBufferHandle_t buffer, size_t offset, size_t amount, 
	void* memory );
