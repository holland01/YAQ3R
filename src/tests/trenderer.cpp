#include "trenderer.h"
#include "../glutil.h"

static const char* gTitle = "I am a floating camera";

TRenderer::TRenderer( void )
    : Test( 1366, 768 ),
	  currentTime( 0.0f ),
      renderer( NULL )
{
}

TRenderer::~TRenderer( void )
{
    camPtr = NULL;
    delete renderer;
}

void TRenderer::Run( void )
{
    renderer->Update( deltaTime );
    renderer->DrawWorld();

	std::stringstream windowTitle;
	// Cap our FPS output at 1000.0f, because anything above that is pretty irrelevant
	windowTitle << gTitle << ": " << glm::min( ( 60.0f / deltaTime ), 1000.0f );

	glfwSetWindowTitle( winPtr, windowTitle.str().c_str() );
}

void TRenderer::Load( void )
{
    if ( !Test::Load( "I am a floating camera" ) )
        return;

	//GL_CHECK( glEnable( GL_BLEND ) );
	//GL_CHECK( glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ) );

    glfwSetInputMode( winPtr, GLFW_STICKY_KEYS, GL_FALSE );

    renderer = new BSPRenderer;
    renderer->Prep();
    renderer->Load( "asset/quake/aty3dm1v2/aty3dm1v2/maps/aty3dm1v2.bsp" );
    //renderer->Load( "asset/quake/railgun_arena/maps/Railgun_Arena.bsp" );

    camPtr = renderer->camera;
}

void TRenderer::OnKeyPress( int key, int scancode, int action, int mods )
{
	Test::OnKeyPress( key, scancode, action, mods );

	if ( action == GLFW_PRESS )
	{
		switch ( key )
		{
		case GLFW_KEY_0:
			renderer->drawDebugInfo = !renderer->drawDebugInfo;
			break;
		case GLFW_KEY_8:
			renderer->gammaCorrectVertexColors = !renderer->gammaCorrectVertexColors;
			break;
		}
	}
}
