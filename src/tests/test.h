#pragma once

#include "common.h"
#include "input.h"
#include "q3bsp.h"
#include "renderer/context_window.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_main.h>

class Q3BspMap;

class Test
{
protected:

	float			deltaTime;	

	InputCamera*    camPtr;

	bool            Load( const char* winTitle );

	float			mouseX;
	float           mouseY;

	float			lastMouseX;
	float			lastMouseY;

	Q3BspMap 		map;

public:

	gContextHandles_t base;	

	Test( int width, int height, bool fullscreen,
	 	const char* bspFilePath, onFinishEvent_t mapReadFinish = nullptr );

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
