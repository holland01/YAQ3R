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
	std::unique_ptr< BSPRenderer > renderer;

	TRenderer( const std::string& mapFilepath );

	~TRenderer( void );

	void 	Load( void );

	bool 	OnInputEvent( SDL_Event* e );

	float 	GetDesiredFPS( void ) const { return TEST_FPS_60; } // 60 FPS
};
