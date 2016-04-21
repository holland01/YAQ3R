#pragma once


#include <emscripten.h>
extern "C" {

struct wApiChunkInfo_t
{
	size_t offset;
	size_t size;
};

static const uint32_t WAPI_TRUE = 1;
static const uint32_t WAPI_FALSE = 0;

}
