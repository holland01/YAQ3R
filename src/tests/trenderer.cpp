#include "trenderer.h"

TRenderer::TRenderer( void )
    : Test( 1366, 768 ),
      renderer( NULL )
{
}

TRenderer::~TRenderer( void )
{
    camPtr = NULL;
    delete renderer;
}

void TRenderer::Load( void )
{
    if ( !Test::Load( "BSPRenderer Test" ) )
        return;

    glfwSetInputMode( winPtr, GLFW_STICKY_KEYS, GL_FALSE );

    renderer = new BSPRenderer;
    renderer->Prep();
    // renderer->Load( "asset/quake/aty3dm1v2/aty3dm1v2/maps/aty3dm1v2.bsp" );
    renderer->Load( "asset/quake/railgun_arena/maps/Railgun_Arena.bsp" );

    camPtr = renderer->camera;
}

void TRenderer::Run( void )
{
    currTime = glfwGetTime();

    renderer->Update( currTime - prevTime );
    renderer->DrawWorld();

    prevTime = currTime;
}
