#include "trenderer.h"
#include "glutil.h"
#include "em_api.h"
#include "extern/gl_atlas.h"

static void OnMapFinish( void* param )
{	
	UNUSED( param );

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

	app->renderer->targetFPS = app->GetDesiredFPS();

	app->camPtr->SetViewOrigin( glm::zero<glm::vec3>() );

	//GStateCheckReport();

	MLOG_INFO(
		"Program Handle: %u",
		app->renderer->debugRender->GetProgram()->GetHandle() 
	);

	app->Exec();
}

TRenderer::TRenderer( const std::string& filepath )
	: Test( 1366, 768, false, filepath.c_str(), OnMapFinish ),
	  moveRateChangeRate( 1.0f ),
	  renderer( nullptr )
{
	O_IntervalLogSetInterval( 5.0f );
}

TRenderer::~TRenderer( void )
{
	camPtr = nullptr;
}

void TRenderer::Run( void )
{
	if ( O_IntervalLogHit() )
	{
		printf( 
			"Origin: %s, FPS: %f\n", 
			glm::to_string( camPtr->ViewData().origin ).c_str(), 
			1.0f / deltaTime 
		);
	}

	O_IntervalLogUpdateFrameTick( deltaTime );

	renderer->Update( deltaTime );

	const viewParams_t& view = camPtr->ViewData();

	GL_CHECK( glBindBuffer( GL_ARRAY_BUFFER, gDeformCache.skyVbo ) );
	GL_CHECK( glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, gDeformCache.skyIbo ) );

	renderer->debugRender->GetProgram()->LoadMat4( "modelToCamera", view.transform );
	renderer->debugRender->GetProgram()->LoadMat4( "cameraToClip", view.clipTransform );

	renderer->debugRender->GetProgram()->Bind();

	renderer->debugRender->GetProgram()->LoadDefaultAttribProfiles();

	GL_CHECK( glDrawElements( GL_TRIANGLES, gDeformCache.numSkyIndices, GL_UNSIGNED_SHORT, nullptr ) );

	//renderer->debugRender->GetProgram()->DisableDefaultAttribProfiles();

	GL_CHECK( glBindBuffer( GL_ARRAY_BUFFER, 0 ) );
	GL_CHECK( glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 ) );

	renderer->debugRender->GetProgram()->Release();
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

bool TRenderer::OnInputEvent( SDL_Event* e )
{
	bool topRet = Test::OnInputEvent( e );

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
					printf( "BSPRenderer: alwaysWriteDepth = %s\n", 
						renderer->alwaysWriteDepth ? "true" : "false" );
					break;
				case SDLK_l:
					renderer->allowFaceCulling = !renderer->allowFaceCulling;
					printf( "BSPRenderer: allowFaceCulling = %s\n", 
						renderer->allowFaceCulling ? "true" : "false" );
				break;
				default:
					break;
			}
			return false;
			break;
		default:
			break;
	}

	return topRet;
}
