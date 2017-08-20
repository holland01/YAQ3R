#include "trenderer.h"
#include "glutil.h"
#include "em_api.h"
#include "extern/gl_atlas.h"

static void OnMapFinish( void* param )
{	
	UNUSED( param );

	TRenderer* app = static_cast< TRenderer* >( gAppTest );

	app->renderer.reset( new BSPRenderer(
		static_cast< float >( app->base.width ), 
		static_cast< float >( app->base.height ),
		*( app->map.get() ) )
	);

	app->renderer->Prep();
	app->renderer->Load( *( app->map->payload ) );

	app->map->payload.reset();
	app->camPtr = app->renderer->camera.get();

	app->renderer->targetFPS = app->GetTargetFPS();

	MLOG_INFO(
		"Program Handle: %u",
		app->renderer->debugRender->GetProgram()->GetHandle() 
	);

	app->Exec();
}

TRenderer::TRenderer( const char* filepath, onFinishEvent_t mapReadFinish )
	:	Test( TEST_VIEW_WIDTH, TEST_VIEW_HEIGHT, false, filepath, mapReadFinish ),
		moveRateChangeRate( 1.0f ),
	  	renderer( nullptr )
{
	O_IntervalLogSetInterval( 5.0f );
}

TRenderer::TRenderer( const std::string& filepath )
	: 	TRenderer( filepath.c_str(), OnMapFinish )
{}

TRenderer::TRenderer( onFinishEvent_t mapReadFinish )
	:	TRenderer( nullptr, mapReadFinish )
{}

TRenderer::~TRenderer( void )
{
	camPtr = nullptr;
}

void TRenderer::LogPeriodically( void ) const
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
}

void TRenderer::Run( void )
{
	LogPeriodically();

	renderer->Update( deltaTime );
	renderer->Render();
}

void TRenderer::Load( void )
{
	if ( !Test::Load( "I am a floating camera" ) )
	{
		MLOG_ERROR(
			"Could not initialize the necessary rendering prerequisites." 
		);
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

//----------------------------------------------------------------

static void IsolatedTestFinish( void* param )
{
	TRendererIsolatedTest* app = static_cast< TRendererIsolatedTest* >( gAppTest );

	app->isolatedRenderer.reset( new RenderBase( TEST_VIEW_WIDTH, TEST_VIEW_HEIGHT ) );
	app->isolatedRenderer->targetFPS = app->GetTargetFPS();

	app->camPtr = app->isolatedRenderer->camera.get();

	gDeformCache.InitSkyData( 512 );

	app->Exec();
}

TRendererIsolatedTest::TRendererIsolatedTest( void )
	: 	TRenderer( IsolatedTestFinish ),
		isolatedRenderer( nullptr )
{
}

TRendererIsolatedTest::~TRendererIsolatedTest( void )
{}

void TRendererIsolatedTest::Run( void )
{
	LogPeriodically();

	isolatedRenderer->Update( deltaTime );

	const viewParams_t& view = camPtr->ViewData();

	GL_CHECK( glBindBuffer( GL_ARRAY_BUFFER, gDeformCache.skyVbo ) );
	GL_CHECK( glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, gDeformCache.skyIbo ) );

	isolatedRenderer->debugRender->GetProgram()->LoadMat4( "modelToCamera", view.transform );
	isolatedRenderer->debugRender->GetProgram()->LoadMat4( "cameraToClip", view.clipTransform );

	isolatedRenderer->debugRender->GetProgram()->Bind();

	isolatedRenderer->debugRender->GetProgram()->LoadDefaultAttribProfiles();

	GL_CHECK( glDrawElements( GL_TRIANGLES, gDeformCache.numSkyIndices, GL_UNSIGNED_SHORT, nullptr ) );

	GL_CHECK( glBindBuffer( GL_ARRAY_BUFFER, 0 ) );
	GL_CHECK( glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 ) );

	isolatedRenderer->debugRender->GetProgram()->Release();
}
