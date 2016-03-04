#pragma once

template < class Tint >
const INLINE Tint NextPower2( Tint x )
{
	x--;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;

	if ( sizeof( x ) >= 2 ) x |= x >> 8;
	if ( sizeof( x ) >= 4 ) x |= x >> 16;
	//if ( sizeof( x ) >= 8 ) x |= x >> 32;

	x++;
	return x;
}

//! TODO: unroll this...
template < class Tint >
INLINE Tint NumBitsSet( Tint x )
{
	const uint16_t maxBitCount = sizeof( x ) * 8;

	Tint numBits = 0;
	for ( uint16_t i = 0; i < maxBitCount; ++i )
		if ( ( x & ( 1 << i ) ) != 0 )
			numBits++;

	return numBits;
}