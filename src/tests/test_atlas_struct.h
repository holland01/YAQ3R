#pragma once

#include "test.h"
#include <set>

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
	float dimX, dimY;
	glm::vec2 origin;
	glm::u8vec4 color;
};

using pointSet_t = std::set< glm::vec2 >;

struct TAtlas : public Test
{
private:
	InputCamera* camera;

	void Run( void );

public:

	std::vector< bounds_t > boundsList;

	TAtlas( void );

	void Load( void );


};
