#include "test.h"
#include "../io.h"
#include "../glutil.h"
#include "renderer/buffer.h"
#include "renderer/context_window.h"

#if defined( EMSCRIPTEN )
#	include <emscripten.h>
#	include "em_api.h"
#endif

Test* gAppTest = nullptr;

static void DefaultOnMapReadFin( void* nullParam )
{
	UNUSED( nullParam );
	gAppTest->base.running = true;
	gAppTest->Exec();
}

static std::string gTmpBspPath;
static onFinishEvent_t gTmpMapReadFinish = nullptr;

static void OnFrameIteration( void )
{
	MLOG_INFO( "%s", "ONE" );

	if ( !gAppTest )
		return;

	MLOG_INFO( "%s", "TWO" );
/*
	SDL_Event e;
	while ( SDL_PollEvent( &e ) )
	{
		if ( !gAppTest->OnInputEvent( &e ) )
			break;
	}
*/
	GL_CHECK( glClearColor( 1.0f, 0.0f, 0.0f, 1.0f ) );
	GL_CHECK( glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) );

//	gAppTest->Run();

	SDL_GL_SwapWindow( gAppTest->base.window );

	float t = GetTimeSeconds();

	gAppTest->deltaTime = ( float )( t - gAppTest->lastTime );
	gAppTest->lastTime = t;
}

Test::Test( int w, int h, bool fullscreen_,
	const char* bspFilePath, onFinishEvent_t mapReadFinish )
	: 	deltaTime( 0.0f ),
		lastTime( 0.0f ),
	  	camPtr( nullptr ),
	  	mouseX( 0.0f ),
	  	mouseY( 0.0f ),
	  	lastMouseX( 0.0f ),
	  	lastMouseY( 0.0f ),
	  	map( new Q3BspMap() ),
	  	base( w, h, fullscreen_ )
{
	if ( bspFilePath )
	{
		gTmpBspPath = bspFilePath;
	}

	if ( mapReadFinish )
	{
		gTmpMapReadFinish = mapReadFinish;
	}
	else
	{
		gTmpMapReadFinish = DefaultOnMapReadFin;
	}

#if defined( EMSCRIPTEN )
	EM_MountFS();
#endif
}

Test::~Test( void )
{
}

bool Test::Load( const char* winName )
{
	if ( !GInitContextWindow( winName, base ) )
	{
		return false;
	}

	GLoadVao();

	if ( !gTmpBspPath.empty() )
	{
		map->Read( gTmpBspPath, 1, gTmpMapReadFinish );
	}

	return true;
}

int Test::Exec( void )
{
	if ( !base.window )
	{
		MLOG_ERROR( "NO window returned! Bailing..." );
		return 1;
	}

#ifdef EMSCRIPTEN
	emscripten_set_main_loop( OnFrameIteration, 0, 1 );
#else

	while( base.running )
	{
		OnFrameIteration();

		deltaTime = ( float )( GetTimeSeconds() - lastTime );
		lastTime = GetTimeSeconds();
	}
#endif

	return 0;
}

bool Test::OnInputEvent( SDL_Event* e )
{
	if ( !e )
		return true;

	switch ( e->type )
	{
		case SDL_KEYDOWN:
			switch ( e->key.keysym.sym )
			{
				case SDLK_ESCAPE:
					base.running = false;
					break;

				case SDLK_F1:
					base.cursorVisible = !base.cursorVisible;

					if ( base.cursorVisible )
					{
						 SDL_SetRelativeMouseMode( SDL_FALSE );
					}
					else
					{
						 SDL_SetRelativeMouseMode( SDL_TRUE );
					}
					 break;

				default:
					if ( camPtr )
					{
						camPtr->EvalKeyPress( e->key.keysym.sym );
					}
					break;
			}

			return false;
			break;

		case SDL_KEYUP:
			if ( camPtr )
				camPtr->EvalKeyRelease( e->key.keysym.sym );
			break;

		case SDL_MOUSEMOTION:
			if ( camPtr )
			{
				camPtr->lastMouse.x = mouseX;
				camPtr->lastMouse.y = mouseY;

				mouseX += ( float )( e->motion.xrel );
				mouseY += ( float )( e->motion.yrel );

				if ( !base.cursorVisible )
					camPtr->EvalMouseMove( mouseX, mouseY );
			}
			break;

		default:
			break;
	}

	return true;
}
