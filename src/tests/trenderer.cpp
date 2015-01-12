#include "trenderer.h"
#include "../glutil.h"

static const char* gTitle = "I am a floating camera";

static const char* railgunArena = "asset/quake/railgun_arena/maps/Railgun_Arena.bsp";
static const char* egyptTemple = "asset/quake/aty3dm1v2/aty3dm1v2/maps/aty3dm1v2.bsp";
static const char* neDuel = "asset/quake/ne_duel/maps/ne_duel.bsp";

TRenderer::TRenderer( void )
    : Test( 1366, 768 ),
	  currentTime( 0.0f ),
      renderer( NULL ),
	  mapFilepath( egyptTemple ),
	  mapLoadFlags( Q3LOAD_ALL )
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
    renderer->Render( mapRenderFlags );

	std::stringstream windowTitle;
	// Cap our FPS output at 1000.0f, because anything above that is pretty irrelevant
	windowTitle << gTitle << ": " << glm::min( ( 60.0f / deltaTime ), 1000.0f );

	glfwSetWindowTitle( winPtr, windowTitle.str().c_str() );
}

void TRenderer::Load( void )
{
    if ( !Test::Load( "I am a floating camera" ) )
        return;

	GL_CHECK( glEnable( GL_DEPTH_TEST ) );
    GL_CHECK( glDepthFunc( GL_LEQUAL ) );
	GL_CHECK( glEnable( GL_FRAMEBUFFER_SRGB ) );
    GL_CHECK( glClearColor( 1.0f, 1.0f, 1.0f, 1.0f ) );

	//GL_CHECK( glEnable( GL_BLEND ) );
	//GL_CHECK( glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ) );

    renderer = new BSPRenderer;
    renderer->Prep();
	renderer->Load( mapFilepath, mapLoadFlags );
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
			mapRenderFlags ^= RENDER_BSP_LIGHTMAP_INFO;
			break;

		case GLFW_KEY_6:
			mapLoadFlags ^= Q3LOAD_TEXTURE_ANISOTROPY;
			break;

		case GLFW_KEY_7:
			mapLoadFlags ^= Q3LOAD_TEXTURE_MIPMAP;
			break;

		case GLFW_KEY_8:
			mapRenderFlags ^= RENDER_BSP_EFFECT;
			break;

		case GLFW_KEY_9:
			useSRGBFramebuffer = !useSRGBFramebuffer;
			if ( useSRGBFramebuffer )
				GL_CHECK( glEnable( GL_FRAMEBUFFER_SRGB ) );
			else
				GL_CHECK( glDisable( GL_FRAMEBUFFER_SRGB ) );
			break;
		}

		if ( key == GLFW_KEY_7 || key == GLFW_KEY_6 )
			renderer->Load( mapFilepath, mapLoadFlags );
	}
}
