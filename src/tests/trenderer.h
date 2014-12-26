#pragma once

#include "test.h"
#include "../renderer.h"

class TRenderer : public Test
{

private:

	float currentTime;

    BSPRenderer* renderer;

    void Run( void );

public:

    TRenderer( void );

    ~TRenderer( void );

    void Load( void );
};
