#pragma once


#include <emscripten.h>
extern "C" {

enum wApiMessage_t
{
	WAPI_READFILE_BEGIN = 0,
	WAPI_READFILE_CHUNK,
	WAPI_READFILE_FINISH
};

struct wApiChunkInfo_t
{
	size_t offset;
	size_t size;
};

static const int WAPI_WORDSIZE = 4;

static inline void wApiRespondPack( wApiMessage_t m )
{
	char buffer[ WAPI_WORDSIZE ];

	for ( int i = 0; i < WAPI_WORDSIZE; ++i )
	{
		buffer[ i ] = ( char ) ( ( ( uint32_t )m >> ( i * 8 ) ) & 0xFF );
	}

	emscripten_worker_respond( buffer, WAPI_WORDSIZE );
}

static inline wApiMessage_t wApiRespondUnpack( char* message )
{
	uint32_t k = 0;

	for ( int i = 0; i < WAPI_WORDSIZE; ++i )
	{
		k |= ( ( uint32_t )message[ i ] ) << ( i * 8 );
	}

	return ( wApiMessage_t )k;
}

}
