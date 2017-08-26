#include "trenderer.h"
#include "glutil.h"
#include "em_api.h"
#include "extern/gl_atlas.h"

static void LoadBspMap( TRenderer* app )
{
	app->renderer.reset( new BSPRenderer(
		static_cast< float >( app->base.width ),
		static_cast< float >( app->base.height ),
		*( app->map.get() ) )
	);

	app->renderer->Prep();
	app->renderer->Load( *( app->map->payload ) );

	app->map->payload.reset();

	app->renderer->targetFPS = app->GetTargetFPS();
}

static void OnMapFinish( void* param )
{
	UNUSED( param );

	TRenderer* app = static_cast< TRenderer* >( gAppTest );

	LoadBspMap( app );

	app->camPtr = app->renderer->camera.get();

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
				case SDLK_j:
					renderer->skyLinearFilter = !renderer->skyLinearFilter;
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
	UNUSED( param );

	TRendererIsolatedTest* app = static_cast< TRendererIsolatedTest* >( gAppTest );

	LoadBspMap( app );

	app->isolatedRenderer.reset( new RenderBase( TEST_VIEW_WIDTH,
		TEST_VIEW_HEIGHT ) );
	app->isolatedRenderer->targetFPS = app->GetTargetFPS();

	if ( app->renderer )
	{
		gla::gen_atlas_layers(
			*( app->renderer->textures[ TEXTURE_ATLAS_DEBUG ] ) );
	}

	app->Load_Quad();

	app->camPtr = app->isolatedRenderer->camera.get();
	app->camPtr->SetViewOrigin( glm::zero< glm::vec3 >() );

	app->Exec();
}

TRendererIsolatedTest::TRendererIsolatedTest( void )
	: 	TRenderer( ASSET_Q3_ROOT"/maps/q3dm13.bsp", IsolatedTestFinish ),
		texture( 0 ),
		gammaPassThrough( 1.0f ),
		isolatedRenderer( nullptr )
{
}

TRendererIsolatedTest::~TRendererIsolatedTest( void )
{}

void TRendererIsolatedTest::Load_Quad( void )
{
	auto LMakeVertex = [ ]( const glm::vec3& position, const glm::vec2& st, const glm::u8vec4& color ) -> bspVertex_t
	{
		return {
			{
				position
			},
			{
				st, st
			},
			{
				0.0f, 0.0f, 0.0f
			},
			color
		};
	};

	std::vector< bspVertex_t > quadVerts( 4 );

	float size = 10.0f;

	quadVerts[ 0 ] = LMakeVertex( { size, -size, 0.0f }, { 1.0f, 0.0f }, { 255, 0, 0, 255 } );
	quadVerts[ 1 ] = LMakeVertex( { size, size, 0.0f }, { 1.0f, 1.0f }, { 255, 255, 0, 255 } );
	quadVerts[ 2 ] = LMakeVertex( { -size, -size, 0.0f }, { 0.0f, 0.0f }, { 0, 0, 255, 255 } );
	quadVerts[ 3 ] = LMakeVertex( { -size, size, 0.0f }, { 0.0f, 1.0f }, { 255, 0, 255, 255 } );

	isolatedRenderer->apiHandles[ 0 ] = GenBufferObject( GL_ARRAY_BUFFER, quadVerts, GL_STATIC_DRAW );
}

void TRendererIsolatedTest::Run_QuadBspEffect( void )
{
	BSPRenderer::drawTuple_t data = std::make_tuple(
		&gDeformCache,
		gDeformCache.skyShader,
		-1,
		-1,
		map->IsTransparentShader( gDeformCache.skyShader )
	);

	auto LDrawCallback = [ this ](
		const void* param,
		const Program& program,
		const shaderStage_t* stage
	) -> void
	{
		UNUSED( param );
		UNUSED( stage );

		MLOG_INFO_ONCE( "Stage Info:\n%s", stage->GetInfoString().c_str() );

		GL_CHECK( glBindBuffer( GL_ARRAY_BUFFER, isolatedRenderer->apiHandles[ 0 ] ) );
		GL_CHECK( glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 ) );

		program.LoadDefaultAttribProfiles();

		GL_CHECK( glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 ) );

		program.DisableDefaultAttribProfiles();

		GL_CHECK( glBindBuffer( GL_ARRAY_BUFFER, renderer->apiHandles[ 0 ] ) );
		GL_CHECK( glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, renderer->apiHandles[ 1 ] ) );
	};

	renderer->DrawEffectPass( data, LDrawCallback );
}

void TRendererIsolatedTest::Run_QuadBsp( void )
{
	Run_Quad();
}

void TRendererIsolatedTest::Run_Quad( void )
{
	const viewParams_t& view = camPtr->ViewData();

	GL_CHECK( glBindBuffer( GL_ARRAY_BUFFER,
		isolatedRenderer->apiHandles[ 0 ] ) );

	const Program* program =
		isolatedRenderer->debugRender->GetProgram( "textured" );

	gla::atlas_t* atlas = renderer->textures[ TEXTURE_ATLAS_DEBUG ].get();
	size_t index = atlas->key_image( 0 );
	gla::atlas_image_info_t info = atlas->image_info( index );

	atlas->bind_to_active_slot( info.layer, 0 );

	program->LoadInt( "sampler0", 0 );
	program->LoadFloat( "gamma", gammaPassThrough );
	program->LoadMat4( "modelToCamera", view.transform );
	program->LoadMat4( "cameraToClip", view.clipTransform );

	program->Bind();

	program->LoadDefaultAttribProfiles();

	GL_CHECK( glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 ) );

	program->DisableDefaultAttribProfiles();

	program->Release();

	GL_CHECK( glBindBuffer( GL_ARRAY_BUFFER, 0 ) );

	atlas->release_from_active_slot( 0 );
}

void TRendererIsolatedTest::Run_Skybox( void )
{
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

void TRendererIsolatedTest::Load( void )
{
	TRenderer::Load();
}

void TRendererIsolatedTest::Run( void )
{
	LogPeriodically();

	if ( renderer && camPtr == renderer->camera.get() )
	{
		renderer->Update( deltaTime );
	}
	else
	{
		camPtr->Update();
	}

	Run_Quad();
}

bool TRendererIsolatedTest::OnInputEvent( SDL_Event* e )
{
	bool ret = TRenderer::OnInputEvent( e );

	switch ( e->type )
	{
		case SDL_KEYDOWN:
			switch ( e->key.keysym.sym )
			{
				case SDLK_m:
					gammaPassThrough += 0.1f;
					gammaPassThrough = glm::clamp( gammaPassThrough,
						1.0f, 2.2f );
					printf( "Gamma: %f\n", gammaPassThrough );
					break;
				case SDLK_n:
					gammaPassThrough -= 0.1f;
					gammaPassThrough = glm::clamp( gammaPassThrough,
						1.0f, 2.2f );
					printf( "Gamma: %f\n", gammaPassThrough );
					break;
			}
			break;
	}

	return ret;
}
