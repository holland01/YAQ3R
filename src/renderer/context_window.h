#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <SDL2/SDL.h>

bool GInitContextWindow( int width, int height, bool fullscreen, const char* winName,
	SDL_Window** sdlWindow, SDL_Renderer** sdlRenderer,
  	SDL_GLContext* sdlContext );

#ifdef __cplusplus
}
#endif
