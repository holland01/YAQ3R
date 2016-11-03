#include "trenderer.h"
#include "glutil.h"
#include "em_api.h"

static void OnMapFinish( void* param )
{
	TRenderer* app = ( TRenderer* ) gAppTest;

	app->renderer.reset( new BSPRenderer(
		( float ) app->base.width,
		( float ) app->base.height,
		*( app->map.get() ) )
	);

	app->renderer->Prep();
	app->renderer->Load( *( app->map->payload ) );

	app->map->payload.reset();
	app->camPtr = app->renderer->camera.get();

	app->Exec();
}

TRenderer::TRenderer( const std::string& filepath )
	: Test( 1366, 768, false, filepath.c_str(), OnMapFinish ),
	  moveRateChangeRate( 0.3f ),
	  renderer( nullptr )
{
}

TRenderer::~TRenderer( void )
{
	camPtr = nullptr;
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
		MLOG_ERROR(
			"Could not initialize the necessary rendering prerequisites." );
		return;
	}

	#ifdef EMSCRIPTEN
		EM_UnmountFS();
	#endif
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
				case SDLK_k:
					renderer->alwaysWriteDepth = !renderer->alwaysWriteDepth;
					break;
				default:
					break;
			}
			break;
		default:
			break;
	}
}
