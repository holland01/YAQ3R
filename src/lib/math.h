#pragma once

template < class Tint >
static INLINE Tint NextPower2( Tint x )
{
	x--;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;

	if ( sizeof( x ) >= 2 ) x |= x >> 8;
	if ( sizeof( x ) >= 4 ) x |= x >> 16;

	x++;
	return x;
}

template < class Tint >
static INLINE Tint NextSquare( Tint l )
{
	Tint closeSquare = NextPower2( l );
	Tint arrayDims = 2;

	while ( arrayDims * arrayDims < closeSquare )
		arrayDims += 2;

	return arrayDims;
}

//! TODO: unroll this...
template < class Tint >
static INLINE Tint NumBitsSet( Tint x )
{
	const uint16_t maxBitCount = sizeof( x ) * 8;

	Tint numBits = 0;
	for ( uint16_t i = 0; i < maxBitCount; ++i )
		if ( ( x & ( 1 << i ) ) != 0 )
			numBits++;

	return numBits;
}

template < typename T >
static INLINE T Inv255( void )
{
	return T( 0.0039215686274509803921568627451 );
}

template < typename T >
static INLINE T Inv128( void )
{
	return T( 0.0078125 );
}

template < typename T >
static INLINE T Inv64( void )
{
	return T( 0.015625 );
}

static INLINE void SetNearFar( glm::mat4& clipTrans, float znear, float zfar )
{
	float invDiff = 1.0f / ( zfar - znear );
	clipTrans[ 2 ][ 2 ] = -1.0f * ( zfar + znear ) * invDiff;
	clipTrans[ 3 ][ 2 ] = -1.0f * ( 2.0f * zfar * znear ) * invDiff;
}
