#include "trenderer.h"
#include "glutil.h"
#include "em_api.h"

static void OnMapFinish( void* param )
{
	UNUSED( param );


}

TRenderer::TRenderer( const std::string& filepath )
	: Test( 1366, 768, false, filepath.c_str() ),
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
		MLOG_ERROR( "Could not initialize the necessary rendering prerequisites." );
		return;
	}

	MLOG_INFO( "The Load Would happen here..." );

/*
	gSamplerHandle_t sampler = GMakeSampler();
	gTextureHandle_t shader; GU_LoadShaderTextures( map, sampler );
	gTextureHandle_t main; GU_LoadMainTextures( map, sampler );

	renderer.reset( new BSPRenderer( ( float ) width,
		( float ) height, map ) );
	renderer->Prep();
	renderer->Load( main, shader, sampler );

	camPtr = renderer->camera.get();
*/

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
