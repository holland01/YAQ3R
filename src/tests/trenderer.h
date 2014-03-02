#pragma once

#include "test.h"
#include "../renderer.h"

class TRenderer : public Test
{

private:

    BSPRenderer* renderer;

    void Run( void );

public:

    TRenderer( void );

    ~TRenderer( void );

    bool Load( void );
};
