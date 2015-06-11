#pragma once

#include "test.h"
#include "../renderer.h"

class TRenderer : public Test
{

private:

	float currentTime;

    BSPRenderer* renderer;

	std::string mapFilepath;

	uint32_t mapLoadFlags;
	uint32_t mapRenderFlags;

	glm::vec3 lightSamplerPos;

    void Run( void );

public:

    TRenderer( void );

    ~TRenderer( void );

    void Load( void );

	void OnKeyPress( int key, int scancode, int action, int mods );
};
