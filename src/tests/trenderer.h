#pragma once

#include "test.h"
#include "../renderer.h"

class TRenderer : public Test
{

private:
	glm::vec3 lightSamplerPos;

	float moveRateChangeRate;

	void Run( void );

public:

	Q3BspMap map;

	std::unique_ptr< BSPRenderer > renderer;

	TRenderer( const std::string& mapFilepath );

	~TRenderer( void );

	void Load( void );

	void OnInputEvent( SDL_Event* e );
};
