#pragma once

#include <random>

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
