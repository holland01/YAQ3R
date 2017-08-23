#pragma once

#include "common.h"
#include "input.h"
#include "q3bsp.h"
#include "renderer/context_window.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_main.h>

class Q3BspMap;

#define TEST_FPS_UNDEFINED -1.0f
#define TEST_FPS_60 0.01666f

#define TEST_VIEW_WIDTH 1366
#define TEST_VIEW_HEIGHT 768

class Test
{
public:
	float			deltaTime;
	float			lastTime;

	InputCamera*    camPtr;

	bool            Load( const char* winTitle );

	float			mouseX;
	float           mouseY;

	float			lastMouseX;
	float			lastMouseY;

	std::unique_ptr< Q3BspMap > 	map;

	gContextHandles_t base;

	Test(
		int width,
		int height,
		bool fullscreen,
	 	const char* bspFilePath,
		onFinishEvent_t mapReadFinish = nullptr
	);

	virtual 		~Test( void );

	int          	Exec( void );

	virtual void 	Load( void ) = 0;

	virtual void 	Run( void ) = 0;

	// ret true: keep polling for events; ret false: break out of poll loop.
	virtual bool 	OnInputEvent( SDL_Event* e );

	void 		 	UpdateTime( void );

	virtual float 	GetDesiredFPS( void ) const { return TEST_FPS_UNDEFINED; }

	GLuint 			MakeRGBATexture( uint8_t* data, GLsizei width, GLsizei height );
};

class IOTest
{
public:
	IOTest( void ) {}
	virtual ~IOTest( void ) {}
	virtual int operator()( void ) = 0;
};

extern Test* gAppTest;
