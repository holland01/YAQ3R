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

// This may seem like overkill, but it's more _secure_
// than just assuming that buffer is of size >= 4
static inline uint32_t WAPI_FetchInt( char* buffer, int size )
{
	if ( !buffer )
	{
		return 0;
	}

	uint32_t x = 0;
	for ( int32_t y = 0; y < size && y < 4; ++y )
	{
		x |= ( uint32_t ) ( buffer[ y ] << ( y << 3 ) );
	}
	return x;
}

}
