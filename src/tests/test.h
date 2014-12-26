#pragma once

#include "../common.h"
#include "../input.h"

class Test
{

protected:

    int             width, height;

    float			deltaTime;

    bool            cursorVisible, running;

    InputCamera*    camPtr;

    GLFWwindow*     winPtr;

    bool            Load( const char* winTitle );

	float			mouseY;
	float			mouseX;

	float			lastMouseX;
	float			lastMouseY;

public:

    Test( int width, int height );

    virtual ~Test( void );

    int          Exec( void );

    virtual void Load( void ) = 0;

    virtual void Run( void ) = 0;

    virtual void OnKeyPress( int key, int scancode, int action, int mods );

    virtual void OnMouseMove( double x, double y );

};

extern Test* gAppTest;
