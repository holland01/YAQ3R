#include "context_window.h"
#include "renderer_local.h"
#include "io.h"

extern "C" {

bool GInitContextWindow( int width, int height, bool fullscreen, const char* winName,
	SDL_Window** sdlWindow, SDL_Renderer** sdlRenderer,
  	SDL_GLContext* sdlContext )
{
	UNUSED( fullscreen );

	SDL_Init( SDL_INIT_VIDEO );
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, G_API_MAJOR_VERSION );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, G_API_MINOR_VERSION );
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, G_API_CONTEXT_PROFILE );

	SDL_CreateWindowAndRenderer( width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN |
		SDL_WINDOW_RESIZABLE, sdlWindow, sdlRenderer );
	SDL_SetWindowTitle( *sdlWindow, winName );

	*sdlContext = SDL_GL_CreateContext( *sdlWindow );

	if ( !( *sdlContext ) )
	{
		MLOG_ERROR( "SDL_Error: %s", SDL_GetError() );
		return false;
	}

#ifndef EMSCRIPTEN
	glewExperimental = true;
	GLenum glewErr = glewInit();
	if ( glewErr != GLEW_OK )
	{
		MLOG_ERROR( "Could not initialize GLEW: %s", glewGetErrorString( glewErr ) );
		return false;
	}
#endif

	SDL_RenderPresent( *sdlRenderer );

	// HACK: error checking is impossible unless this happens;
	// apparently an invalid enumeration exists - possibly
	// in SDL or GLEW's initialization...

	glGetError();

	return true;
}

}
