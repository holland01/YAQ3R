#pragma once


#include <emscripten.h>
extern "C" {

struct wApiChunkInfo_t
{
	size_t offset;
	size_t size;
};

static const uint32_t WAPI_ERROR = 0xFFFFFFFF;
static const uint32_t WAPI_TRUE = 1;
static const uint32_t WAPI_FALSE = 0;

// This may seem like overkill, but it's more _secure_
// than just assuming that buffer is of size >= 4
static inline uint32_t WAPI_FetchBool( char* buffer, int ofs, int size )
{
	if ( !buffer ) return 0;
	if ( ofs >= ( size - 3 ) ) return WAPI_ERROR;

	// We do have to be this pedantic with the & 0xFF: bad things
	// can happen in produced emscripten code if we don't throw that in there.
	uint32_t x = 0;
	for ( int32_t y = 0; y < size && y < 4; ++y )
	{
		x |= ( ( uint32_t )( buffer[ y ] << ( y << 3 ) ) & 0xFF );
	}
	return x;
}

static inline uint32_t WAPI_Fetch16( char* buffer, int ofs, int size )
{
	if ( !buffer ) return WAPI_ERROR;
	if ( ofs >= ( size - 1 ) ) return WAPI_ERROR;


	uint32_t x = ( ( uint32_t )buffer[ ofs ] ) & 0xFF;
	uint32_t y = ( ( uint32_t )buffer[ ofs + 1 ] ) & 0xFF;

	return x | ( y << 8 );
}

}
