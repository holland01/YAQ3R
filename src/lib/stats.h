#pragma once

#include "common.h"

template < class Tint >
struct median_t
{
	std::vector< Tint > store;

	void Insert( Tint v )
	{
		if ( store.empty() )
		{
			store.push_back( v );
			return;
		}

		Tint i = 0;
		Tint k = store[ 0 ];

		while ( k < v && i < store.size() )
		{
			k = store[ ++i ];
		}

		if ( k > v )
		{
			store.insert( store.begin() + i, v );
		}
		else if ( k < v )
		{
			if ( i == store.size() )
			{
				store.push_back( v );
			}
			else
			{
				store.insert( store.begin() + i + 1, v );
			}
		}
	}

	Tint GetMedian( void ) const
	{
		return store[ store.size() >> 1 ];
	}

	Tint SumToMedian( void ) const
	{
		uint16_t exclusive = store.size() >> 1;
		uint16_t sum = 0;
		for ( uint16_t i = 0; i < exclusive; ++i )
		{
			sum += store[ i ];
		}
		return sum;
	}

	Tint GetHigh( void ) const
	{
		return store[ store.size() - 1 ];
	}
};

