#pragma once

#include "test.h"
#include "../renderer.h"

class TRenderer : public Test
{

private:

	BSPRenderer* renderer;

	std::string mapFilepath;

	glm::vec3 lightSamplerPos;

	float moveRateChangeRate;

	void Run( void );

public:

	TRenderer( void );

	~TRenderer( void );

	void Load( void );

	void OnInputEvent( SDL_Event* e );
};
