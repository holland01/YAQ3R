#pragma once

#include "test.h"
#include "../renderer.h"

class TRenderer : public Test
{

private:

    BSPRenderer* renderer;

	std::string mapFilepath;

	uint32_t mapLoadFlags;

	glm::vec3 lightSamplerPos;

    void Run( void );

public:

    TRenderer( void );

    ~TRenderer( void );

    void Load( void );
};
