#pragma once

#include "common.h"

// Tnum can be any kind of numeric class;
// we reserve Treal for calculations involving
// real numbers (duh), considering that we may
// just want to store and sort integers, while
// performing floating point calculations on them
// as necessary
template < class Tnum, class Treal = float >
struct stats_t
{
	// vector is the most versatile data structure for our purposes here:
	// we can write code to ensure that its values are a) ordered and b)
	// unique, while still maintaining the ability to do killer things
	// like have read O(1) access and choose our insertion points
	std::vector< Tnum > store;

	bool InsertOrderedUnique( Tnum v, Tnum* slot = nullptr )
	{
		if ( store.empty() )
		{
			store.push_back( v );
			return true;
		}

		Tnum i = 0;
		Tnum k = store[ 0 ];

		while ( k < v && i < store.size() )
		{
			k = store[ ++i ];
		}

		if ( slot )
		{
			*slot = i;
		}

		// vector::insert will insert a value
		// before the specified index.
		if ( k > v )
		{
			store.insert( store.begin() + i, v );
			return true;
		}
		else if ( k < v ) // i == store.size(), so we append
		{
			store.push_back( v );
			return true;
		}
		// else k == v, and we don't want to insert it
		// since we want unique only...

		return false;
	}

	void InsertOrdered( Tnum v )
	{
		Tnum slot;
		if ( !InsertOrderedUnique( v, &slot ) )
		{
			assert( store[ slot ] == v );
			store.insert( store.begin() + slot, v );
		}
	}

	Treal Average( void ) const
	{
		return Treal( Sum() ) / Treal( store.size() );
	}

	Treal Deviation( Tnum target ) const
	{
		Treal sum = 0;
		for ( uint16_t i = 0; i < store.size(); ++i )
		{
			sum += std::pow( Treal( store[ i ] - target ), Treal( 2 ) );
		}
		return std::sqrt( sum / Treal( target ) );
	}

	Treal ZScore( Tnum target, Treal average, Treal deviation ) const
	{
		Treal rtarg( target );
		return ( rtarg - average ) / deviation;
	}

	Treal ZScore( Tnum target, Treal* outDeviation = nullptr )
	{
		Treal average = Average();
		Treal stdDev = Deviation( average );

		if ( outDeviation )
		{
			*outDeviation = stdDev;
		}

		float z = ZScore( target, average, stdDev );

		return z;
	}

	Treal GetMedian( void ) const
	{
		uint16_t m = store.size() >> 1;
		if ( store.size() % 2 == 0 )
		{
			return Treal( store[ m ] + store[ m - 1 ] ) * Treal( 0.5 );
		}
		return Treal( store[ m ] );
	}

	Tnum Sum( void ) const
	{
		Tnum sum = 0;
		for ( uint16_t i = 0; i < store.size(); ++i )
		{
			sum += store[ i ];
		}
		return sum;
	}

	Tnum GetHigh( void ) const
	{
		return store[ store.size() - 1 ];
	}
};

