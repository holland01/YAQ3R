#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <SDL2/SDL.h>

struct gContextHandles_t
{
	int width;
	int height;
	bool fullscreen;	

	SDL_GLContext context = nullptr;
	SDL_Renderer* renderer = nullptr;
	SDL_Window* window = nullptr;

	~gContextHandles_t( void )
	{
		if ( renderer ) SDL_DestroyRenderer( renderer );
		if ( window ) SDL_DestroyWindow( window );
	} 
};

bool GInitContextWindow( );

#ifdef __cplusplus
}
#endif
