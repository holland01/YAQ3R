#pragma once

#include "commondef.h"
#include <emscripten.h>
#include <stdlib.h>

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
static inline uint32_t WAPI_Fetch32( char* buffer, int size, int ofs )
{
	if ( !buffer ) return WAPI_FALSE; // <- not necessarily an error, because
									  // this may be called knowing that
									  // the buffer may not exist

	int32_t diff = size - 4;
	if ( ( ofs ) > diff )
	{
		return WAPI_FALSE;
	}

	// We do have to be this pedantic with the & 0xFF: bad things
	// can happen in produced emscripten code if we don't throw that in there.
	uint32_t x = 0;
	for ( int32_t y = 0; y < 4; ++y )
	{
		x |= ( ( uint32_t )( buffer[ y ] ) & 0xFF ) << ( y << 3 );
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

static inline uint32_t WAPI_StrEquals( register const char* a,
	register const char* b, int n, int* stopIndex )
{
	int k = 0;
	while (	a[k] == b[k] && k < n )
	{
		k++;
	}

	if ( stopIndex )
	{
		*stopIndex = k;
	}

	return k == n;
}

// 8 byte key: _TOKEXXX, where _ is a mandatory prefix,
// TOKE is any four letters used to identify the key,
// and XXX is a hexadecimal value specifying the amount of entries for that key
// entries are 4 bytes each, but are required to be 8 byte aligned.
// So, a key with one entry
// will still require an extra 8 bytes.
// if 4 bytes of memory isn't necessary for a given entry, a freespace '_'
// is used in areas where the remaining amount isn't needed

#define ZERO_BYTE ( ( uint32_t )'0' )
#define NINE_BYTE ( ( uint32_t )'9' )

#define A_BYTE ( ( uint32_t )'A' )
#define F_BYTE ( ( uint32_t )'F' )

static inline uint32_t WAPI_FetchHex( char* val, int n )
{
	if ( n > 0x10 )
	{
		return -1;
	}

	uint32_t ret = 0;
	for ( int i = 0; i < n; ++i )
	{
		uint32_t c = ( uint32_t ) toupper( val[i] );

		uint32_t shiftw = ( n - i - 1 ) * 4;

		if ( ZERO_BYTE <= c && c <= NINE_BYTE )
		{
			ret |= shiftw << ( c - ZERO_BYTE );
		}
		else if  ( A_BYTE <= c && c <= F_BYTE )
		{
			ret |= shiftw << ( c - ( A_BYTE - 0xA ) );
		}
		else
		{
			return -1;
		}
	}

	return ret;
}

static inline uint32_t WAPI_CalcCheckSum( char* val, int n )
{
	uint32_t byte = 0;

	for ( int i = 0; i < n; ++i )
	{
		byte = ( byte + ( uint32_t ) val[ i ] ) & 0xFF;
	}

	return ( ( byte ^ 0xFF ) + 1 ) & 0xFF;
}

// ERRORS:
// - when any of the specified sizes are less than or equal to zero
// - if either the dest buffer or key buffer is null
// - when any of the specified sizes are not aligned to 4 bytes
// - if destSize is less than the key's entry length

//
// FORMAT:
// - Each element is 4 bytes (including keys).
// - First four bytes is the length of the first key/entry pair.
// - Length of first key/entry pair is m = 2^(ceil(log2(n))), where n is the
// 		actual number of values which pertain to that key/value pair
//	(including the key itself)
// - For any key K at some index i in the list, its corresponding length
//	L_i will be:
// 		L_i = 2^(log2(L_0) + i); i > 0, L_i != L_0, and 2 <= L_0
// - Unused elements in a given segment (where a segment is some area of memory
//	corresponding to K with L_i - 1 values)
// 		are blocked out with the letters BLNK (blank)
// - Any entry which contains at least 1 byte will have its remaining unset
//	bytes set to '_'

// Map this iterative approach to a binary search.
// f(i) = 2^i
// f(i + 1) = 2^(i + 1)
// f((L + R) >> 1) = 2^((L+R) >> 1)
//

/*
static inline uint32_t WAPI_FetchKey( char* dest, int destSize, const char* key,
	int keySize, char* buffer, int bufferSize )
{
	if ( !buffer ) return WAPI_FALSE;

	if ( !dest || !key ) return WAPI_ERROR;

	if ( keySize <= 0 || bufferSize <= 0 || destSize <= 0 ) return WAPI_ERROR;

	if ( !IS_ALIGN4( bufferSize ) || !IS_ALIGN4( destSize )
		|| !IS_ALIGN4( keySize ) )
	{
		return WAPI_ERROR;
	}

	uint32_t pow2 = WAPI_Fetch32( buffer, 0 );

	int l = 4;
	int r = bufferSize - 1;

	while ( l <= r )
	{

	}

	for ( int i = 4; i < bufferSize; )
	{
		if ( strncmp( key, buffer + i, 4 ) == 0 )
		{
			int vlength = pow2 - 4;

			if ( destSize < vlength )
			{
				return WAPI_ERROR;
			}

			memcpy( dest, buffer + i + 4, vlength );

			return WAPI_TRUE;
		}

		i += pow2;
		pow2 <<= 1;
	}

	return WAPI_FALSE;
}
*/
}
