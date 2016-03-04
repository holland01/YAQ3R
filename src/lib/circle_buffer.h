#pragma once

#include "common.h"
#include "math.h"
#include <array>

template < class T, uint16_t N >
struct circleBufferU16_t
{
	const uint32_t LOW_MASK = NextPower2( N ) - 1;
	const uint32_t NUM_BITS = NumBitsSet( LOW_MASK );
	const uint32_t HIGH_MASK = LOW_MASK << NUM_BITS;

	std::array< T, N > buffer;
	mutable uint32_t pointer;

	circleBufferU16_t( const T& fill = T() )
		: pointer( 0 )
	{
		buffer.fill( fill );
	}

	uint16_t WritePointer( void ) const { return uint16_t( pointer & LOW_MASK ); }
	uint16_t ReadPointer( void ) const { return uint16_t( ( pointer & HIGH_MASK ) >> NUM_BITS ); }

	void SetWritePointer( uint32_t x )
	{
		pointer = ( pointer & HIGH_MASK ) + x;
	}

	void SetReadPointer( uint32_t x ) const
	{
		pointer = ( pointer & LOW_MASK ) + ( x << NUM_BITS );
	}

	const T& Read( void ) const
	{
		uint32_t r = ReadPointer();
		if ( r == N )
			r = 0;
		const T& ret = buffer[ r ];
		r++;
		SetReadPointer( r );
		return ret;
	}

	void Write( const T& v )
	{
		uint32_t w = WritePointer();
		if ( w == N )
			w = 0;
		buffer[ w ] = v;
		w++;
		SetWritePointer( w );
	}
};



