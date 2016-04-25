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

static void OnMapReadFin( void* nullParam )
{
	UNUSED( nullParam );

	gAppTest->Load();
	gAppTest->Exec();
}

static void OnFrameIteration( void )
{
	if ( !gAppTest )
		return;

	GL_CHECK( glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) );

	gAppTest->Run();

	SDL_GL_SwapWindow( gAppTest->sdlWindow );

	SDL_Event e;
	while ( SDL_PollEvent( &e ) )
	{
		gAppTest->OnInputEvent( &e );
	}
}

Test::Test( int w, int h, bool fullscreen_,
	const char* bspFilePath, onFinishEvent_t mapReadFinish )
	: width( w ), height( h ),
	  deltaTime( 0.0f ),
	  fullscreen( fullscreen_ ),
	  cursorVisible( true ),
	  running( false ),
	  useSRGBFramebuffer( true ),
	  camPtr( nullptr ),
	  sdlRenderer( nullptr ),
	  sdlContext( nullptr ),
	  mouseX( 0.0f ),
	  mouseY( 0.0f ),
	  lastMouseX( 0.0f ),
	  lastMouseY( 0.0f ),
	  sdlWindow( nullptr )
{
#if defined( EMSCRIPTEN )
	EM_MountFS();
#endif
	if ( bspFilePath )
	{
		if ( !mapReadFinish )
		{
			mapReadFinish = OnMapReadFin;
		}
		map.Read( std::string( bspFilePath ), 1, mapReadFinish );
	}
}

Test::~Test( void )
{
	if ( sdlWindow )
		SDL_DestroyWindow( sdlWindow );

	if ( sdlRenderer )
		SDL_DestroyRenderer( sdlRenderer );

	KillSysLog();
}

bool Test::Load( const char* winName )
{
	if ( !GInitContextWindow( width, height, fullscreen, winName,
	 	&sdlWindow, &sdlRenderer, &sdlContext ) )
	{
		return false;
	}

	GLoadVao();

	running = true;

	InitSysLog();

	return true;
}

int Test::Exec( void )
{
	if ( !sdlWindow )
		return 1;

#ifdef EMSCRIPTEN
	emscripten_set_main_loop( OnFrameIteration, 0, 1 );
#else
	float lastTime = 0.0f;

	while( running )
	{
		OnFrameIteration();

		deltaTime = ( float )( GetTimeSeconds() - lastTime );
		lastTime = GetTimeSeconds();
	}
#endif

	return 0;
}

void Test::OnInputEvent( SDL_Event* e )
{
	if ( !e )
		return;

	switch ( e->type )
	{
		case SDL_KEYDOWN:
			switch ( e->key.keysym.sym )
			{
				case SDLK_ESCAPE:
					running = false;
					break;

				case SDLK_F1:
					cursorVisible = !cursorVisible;

					if ( cursorVisible )
						 SDL_SetRelativeMouseMode( SDL_FALSE );
					else
						 SDL_SetRelativeMouseMode( SDL_TRUE );
					break;

				default:
					if ( camPtr )
						camPtr->EvalKeyPress( e->key.keysym.sym );
					break;
			}
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

				if ( !cursorVisible )
					camPtr->EvalMouseMove( mouseX, mouseY );
			}
			break;

		default:
			break;
	}
}
