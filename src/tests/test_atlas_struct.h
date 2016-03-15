#pragma once

#include "test.h"
#include <set>
#include <memory>

namespace std {
	template <>
	struct less< glm::vec2 >
	{
		bool operator()( const glm::vec2& a, const glm::vec2& b ) const
		{
			return glm::length( a ) < glm::length( b );
		}
	};
}

struct bounds_t
{
	uint16_t dimX, dimY;
	glm::vec2 origin;
	glm::u8vec4 color;
	bool rot = false;
};

struct median_t
{
	std::vector< uint16_t > store;

	void Insert( uint16_t v )
	{
		if ( store.empty() )
		{
			store.push_back( v );
			return;
		}

		uint16_t i = 0;
		uint16_t k = store[ 0 ];

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

	uint16_t Get( void )
	{
		return store[ store.size() >> 1 ];
	}
};

struct tree_t;

using pointSet_t = std::set< glm::vec2 >;

struct TAtlas : public Test
{
private:
	InputCamera* camera;

	std::unique_ptr< tree_t > tree;

	void Run( void );

public:

	std::vector< bounds_t > boundsList;

	TAtlas( void );

	void Load( void );


};
