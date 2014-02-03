#pragma once

#include "common.h"
#include "renderer.h"

static const int KEY_COUNT = 6;

class Input
{
public:
    Input( void );

    void EvalKeyPress( int key );
    void EvalKeyRelease( int key );
    void EvalMouseMove( float x, float y );

    void UpdatePass( RenderPass& pass );

    const RenderPass& LastPass() const { return lastPass; }

private:

    float           mouseX, mouseY;

    byte            keysPressed[ KEY_COUNT ];

    RenderPass      lastPass;
};
