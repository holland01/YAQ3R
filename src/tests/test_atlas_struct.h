#pragma once

#include "test.h"
#include "lib/atlas_gen.h"
#include "renderer/texture.h"

struct TAtlas : public Test
{
private:
	InputCamera* camera;

	std::vector< gImageParams_t > images;

	void Run( void );

public:

	TAtlas( void );

	void Load( void );


};
