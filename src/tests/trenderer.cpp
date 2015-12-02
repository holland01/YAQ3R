#include "trenderer.h"
#include "../glutil.h"

static const char* gTitle = "I am a floating camera";

static const char* railgunArena = "asset/quake/railgun_arena/maps/Railgun_Arena.bsp";
static const char* egyptTemple = "asset/quake/aty3dm1v2/aty3dm1v2/maps/aty3dm1v2.bsp";
static const char* neDuel = "asset/quake/ne_duel/maps/ne_duel.bsp";
static const char* dm1 = "asset/stockmaps/maps/q3dm15.bsp";
static const char* tourney = "asset/stockmaps/maps/q3tourney2.bsp";

TRenderer::TRenderer( void )
    : Test( 1920, 1080, false ),
	  currentTime( 0.0f ),
      renderer( nullptr ),
	  mapFilepath( railgunArena ),
	  mapLoadFlags( Q3LOAD_ALL )
{
}

TRenderer::~TRenderer( void )
{
    camPtr = nullptr;
    delete renderer;
}

void TRenderer::Run( void )
{
    renderer->Update( deltaTime );
    renderer->Render();

	std::stringstream windowTitle;
	// Cap our FPS output at 1000.0f, because anything above that is pretty irrelevant
	windowTitle << gTitle << ": " << glm::min( renderer->CalcFPS(), 1000.0f );

	glfwSetWindowTitle( winPtr, windowTitle.str().c_str() );
}

void TRenderer::Load( void )
{
    if ( !Test::Load( "I am a floating camera" ) )
        return;

    renderer = new BSPRenderer( ( float ) width, ( float ) height );
    renderer->Prep();
	renderer->Load( mapFilepath, mapLoadFlags );
    camPtr = renderer->camera;
}

void TRenderer::OnKeyPress( int key, int scancode, int action, int mods )
{
	Test::OnKeyPress( key, scancode, action, mods );

	static const float ofsStep = 20.0f;

	if ( action == GLFW_PRESS )
	{
		switch ( key )
		{
		case GLFW_KEY_V:
			if ( renderer->curView == VIEW_MAIN )
			{
				renderer->curView = VIEW_LIGHT_SAMPLE;
			} 
			else
			{
				renderer->curView = VIEW_MAIN;
			}
			break;
		case GLFW_KEY_0:
			useSRGBFramebuffer = !useSRGBFramebuffer;
			
			if ( useSRGBFramebuffer )
			{
				GL_CHECK( glEnable( GL_FRAMEBUFFER_SRGB ) );
			}
			else
			{
				GL_CHECK( glDisable( GL_FRAMEBUFFER_SRGB ) );
			}
			break;

		case GLFW_KEY_1:
			mapLoadFlags ^= Q3LOAD_TEXTURE_ANISOTROPY;
			break;

		case GLFW_KEY_2:
			mapLoadFlags ^= Q3LOAD_TEXTURE_MIPMAP;
			break;

		case GLFW_KEY_6:
			mapLoadFlags ^= RENDER_BSP_USE_TCMOD;
			break;	

		case GLFW_KEY_7:
			mapRenderFlags ^= RENDER_BSP_ALWAYS_POLYGON_OFFSET;
			break;

		case GLFW_KEY_8:
			mapRenderFlags ^= RENDER_BSP_EFFECT;
			break;

		case GLFW_KEY_9:
			mapRenderFlags ^= RENDER_BSP_LIGHTMAP_INFO;
			break;
		}

		// Perform a reload only if we've pressed keys 0-9
		if ( 48 <= key && key <= 57 )
		{
			renderer->Load( mapFilepath, mapLoadFlags );
		}
	}
}
