#pragma once

#include <random>
#include "common.h"

template < class Tnum >
class Random
{
	typedef typename std::conditional< std::is_floating_point< Tnum >::value,
		std::uniform_real_distribution< Tnum >,
		std::uniform_int_distribution< Tnum > >::type randomDist_t;

	mutable std::random_device device;
	mutable std::mt19937 engine;
	mutable randomDist_t dist;

public:
	Random( Tnum min, Tnum max )
		: engine( device() ),
		  dist( min, max )
	{
	}

	Tnum operator()( void ) const
	{
		return dist( engine );
	}
};

static INLINE glm::u8vec4 RandUniqueColor( Random< uint8_t >& colorGen,
						std::vector< glm::u8vec4 >& table )
{
	while ( true )
	{
		glm::u8vec4 c( colorGen(), colorGen(), colorGen(), 255 );

		bool found = false;

		for ( const glm::u8vec4& color: table )
		{
			if ( c == color )
			{
				found = true;
				break;
			}
		}

		if ( !found )
		{
			table.push_back( c );
			return c;
		}
	}

	return glm::u8vec4( 0 ); // <___<
}
