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
	bool cursorVisible;
	bool running;
	bool useSRGBFramebuffer;

	SDL_GLContext context;
	SDL_Renderer* renderer;
	SDL_Window* window;

	gContextHandles_t( int width_, int height_, bool fullscreen_ )
		: 	width( width_ ),
			height( height_ ),
			fullscreen( fullscreen_ ),
	  		cursorVisible( true ),
	  		running( false ),
	  		useSRGBFramebuffer( true ),
			context( nullptr ),
			renderer( nullptr ),
			window( nullptr )
	{
	}

	~gContextHandles_t( void )
	{
		if ( renderer )
		{
			SDL_DestroyRenderer( renderer );
		}
		
		if ( window )
		{
			SDL_DestroyWindow( window );
		}
	} 
};

void GPrintContextInfo( void );

bool GInitContextWindow( const char* title, gContextHandles_t& context );
#ifdef __cplusplus
}
#endif
