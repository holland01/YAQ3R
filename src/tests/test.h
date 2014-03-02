#pragma once

#include "../common.h"
#include "../input.h"

class Test
{

protected:

    int             width, height;

    double          currTime, prevTime;

    bool            cursorVisible, running;

    Camera*         camPtr;

    GLFWwindow*     winPtr;

    bool            Load( const char* winTitle );

public:

    Test( int width, int height );

    virtual ~Test( void );

    int          Exec( void );

    virtual bool Load( void ) = 0;

    virtual void Run( void ) = 0;

    virtual void OnKeyPress( int key, int scancode, int action, int mods );

    virtual void OnMouseMove( double x, double y );

    void         Shutdown( void );

};

extern Test* gAppTest;
