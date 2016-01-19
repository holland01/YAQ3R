#include "test.h"
#include "../io.h"
#include "../glutil.h"

Test* gAppTest = NULL;

static void OnKeyPressWrapper( GLFWwindow* w, int key, int scancode, int action, int mods )
{
    if ( gAppTest )
        gAppTest->OnKeyPress( key, scancode, action, mods );
}

static void OnMouseMoveWrapper( GLFWwindow* w, double x, double y )
{
    if ( gAppTest )
        gAppTest->OnMouseMove( x, y );
}

Test::Test( int w, int h, bool fullscreen_ )
    : width( w ), height( h ),
      deltaTime( 0.0f ),
	  fullscreen( fullscreen_ ),
      cursorVisible( true ), 
	  running( false ), 
	  useSRGBFramebuffer( true ),
      camPtr( nullptr ),
      winPtr( nullptr ),
	  mouseX( 0.0f ),
	  mouseY( 0.0f ),
	  lastMouseX( 0.0f ),
	  lastMouseY( 0.0f )
{
}

Test::~Test( void )
{
    if ( winPtr )
        glfwDestroyWindow( winPtr );

    glfwTerminate();
    KillSysLog();
}

bool Test::Load( const char* winName )
{
    if ( !glfwInit() )
        return false;

    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 0 );
    //glfwWindowHint( GLFW_CLIENT_API, GLFW_OPENGL_ES_API );
    //  glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
	//glfwWindowHint( GLFW_SRGB_CAPABLE, GL_TRUE );

	GLFWmonitor* monitor = fullscreen? glfwGetPrimaryMonitor(): nullptr;

    winPtr = glfwCreateWindow( width, height, winName, monitor, nullptr );

    if ( !winPtr )
        return false;

    glfwMakeContextCurrent( winPtr );

	glfwSetKeyCallback( winPtr, OnKeyPressWrapper );
    glfwSetCursorPosCallback( winPtr, OnMouseMoveWrapper );

	glfwSetInputMode( winPtr, GLFW_STICKY_KEYS, GL_FALSE );

    glewExperimental = true;
    GLenum response = glewInit();

    if ( response != GLEW_OK )
    {
        printf( "Could not initialize GLEW! %s", glewGetErrorString( response ) );
        return false;
    }

    MyPrintf( "Init", "OpenGL Version: %i.%i",
              glfwGetWindowAttrib( winPtr, GLFW_CONTEXT_VERSION_MAJOR ),
              glfwGetWindowAttrib( winPtr, GLFW_CONTEXT_VERSION_MINOR ) );


    // GLEW-dependent OpenGL error is pushed on init. This is a temporary hack to just pop
    // that error off the error stack.
    glGetError();

    running = true;

    InitSysLog();

    return true;
}

int Test::Exec( void )
{
    if ( !winPtr )
        return 1;

	float lastTime = 0.0f;

    while( running && !glfwWindowShouldClose( winPtr ) )
    {
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		deltaTime = ( float )glfwGetTime() - lastTime;

        Run();

        glfwSwapBuffers( winPtr );
        glfwPollEvents();

		lastTime = ( float )glfwGetTime();
    }

    return 0;
}

void Test::OnKeyPress( int key, int scancode, int action, int mods )
{
    switch ( action )
    {
        case GLFW_PRESS:
            switch ( key )
            {
                case GLFW_KEY_ESCAPE:
                    running = false;
                    break;

                case GLFW_KEY_F1:
                    cursorVisible = !cursorVisible;

                    if ( cursorVisible )
                        glfwSetInputMode( winPtr, GLFW_CURSOR, GLFW_CURSOR_NORMAL );
                    else
                        glfwSetInputMode( winPtr, GLFW_CURSOR, GLFW_CURSOR_DISABLED );

                    break;

                default:
					if ( camPtr )
                        camPtr->EvalKeyPress( key );
                    break;
            }
            break;

        case GLFW_RELEASE:
			if ( camPtr )
                camPtr->EvalKeyRelease( key );
            break;

        default:
            break;
    }
}

void Test::OnMouseMove( double x, double y )
{
	if ( camPtr )
	{
		camPtr->lastMouse.x = mouseX;
		camPtr->lastMouse.y = mouseY;

		mouseX = ( float ) x;
		mouseY = ( float ) y;

		if ( !cursorVisible )
			camPtr->EvalMouseMove( mouseX, mouseY );
	}
}
