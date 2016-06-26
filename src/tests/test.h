#pragma once

#include "common.h"
#include "input.h"
#include "q3bsp.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_main.h>

class Q3BspMap;

struct gContextHandles_t
{
	SDL_GLContext context = nullptr;
	SDL_Renderer* renderer = nullptr;
	SDL_Window* window = nullptr;

	~gContextHandles_t( void )
	{
		if ( renderer ) SDL_DestroyRenderer( renderer );
		if ( window ) SDL_DestroyWindow( window );
	}
};

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

	bool            Load( const char* winTitle );

	float			mouseX;
	float           mouseY;

	float			lastMouseX;
	float			lastMouseY;

	Q3BspMap 		map;

public:

	gContextHandles_t context;	

	Test( int width, int height, bool fullscreen,
	 	const char* readFilePath, onFinishEvent_t mapReadFinish = nullptr );

	virtual ~Test( void );

	int          Exec( void );

	virtual void Load( void ) = 0;

	virtual void Run( void ) = 0;

	virtual void OnInputEvent( SDL_Event* e );
};

class IOTest
{
public:
	IOTest( void ) {}
	virtual ~IOTest( void ) {}
	virtual int operator()( void ) = 0;
};

extern Test* gAppTest;
