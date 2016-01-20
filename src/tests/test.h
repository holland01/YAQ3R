#pragma once

#include "../common.h"
#include "../input.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_main.h>

class Test
{
protected:

    int             width, height;

    float			deltaTime;

    bool            fullscreen,
					cursorVisible, 
					running, 
					useSRGBFramebuffer;

    InputCamera*    camPtr;

	SDL_Window*		sdlWindow;

	SDL_Renderer*	sdlRenderer;

	SDL_GLContext	sdlContext;

    bool            Load( const char* winTitle );

	float			mouseX;
    float           mouseY;

	float			lastMouseX;
	float			lastMouseY;

public:

    Test( int width, int height, bool fullscreen );

    virtual ~Test( void );

    int          Exec( void );

    virtual void Load( void ) = 0;

    virtual void Run( void ) = 0;

	virtual void OnInputEvent( SDL_Event* e );
};

extern Test* gAppTest;
