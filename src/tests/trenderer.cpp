#include "trenderer.h"
#include "../glutil.h"

TRenderer::TRenderer( void )
	: Test( 1920, 1080, false ),
	  renderer( nullptr ),
	  mapFilepath( "asset/stockmaps/maps/q3dm1.bsp" ),
	  moveRateChangeRate( 0.3f )
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
}

void TRenderer::Load( void )
{
	if ( !Test::Load( "I am a floating camera" ) )
	{
		MLOG_ERROR( "Could not initialize the necessary rendering prerequisites." );
		return;
	}

	renderer = new BSPRenderer( ( float ) width, ( float ) height );
	renderer->Prep();
	renderer->Load( mapFilepath );
	camPtr = renderer->camera;
}

void TRenderer::OnInputEvent( SDL_Event* e )
{
	Test::OnInputEvent( e );

	switch ( e->type )
	{
		case SDL_KEYDOWN:
			switch ( e->key.keysym.sym )
			{
				case SDLK_UP:
					if ( camPtr )
					{
						camPtr->moveStep += moveRateChangeRate;
					}
					break;
				case SDLK_DOWN:
					if ( camPtr )
					{
						camPtr->moveStep -= moveRateChangeRate;
					}
					break;
				case SDLK_RIGHT:
					moveRateChangeRate += 0.1f;
					break;
				case SDLK_LEFT:
					moveRateChangeRate -= 0.1f;
					break;
				default:
					break;
			}
			break;
		default:
			break;
	}
}
