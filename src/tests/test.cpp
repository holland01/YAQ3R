#include "test.h"
#include "../log.h"

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

Test::Test( int w, int h )
    : width( w ), height( h ),
      currTime( 0.0 ), prevTime( 0.0 ),
      cursorVisible( true ), running( false ),
      camPtr( NULL ),
      winPtr( NULL )
{
}

Test::~Test( void )
{
    if ( winPtr )
        glfwDestroyWindow( winPtr );

    // Set to NULL in child dtor to avoid deletion.
    if ( camPtr )
        delete camPtr;

    glfwTerminate();
    KillSysLog();
}

bool Test::Load( const char* winName )
{
    if ( !glfwInit() )
        return false;

    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 2 );

    winPtr = glfwCreateWindow( width, height, winName, NULL, NULL );

    if ( !winPtr )
        return false;

    glfwMakeContextCurrent( winPtr );

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

    glEnable( GL_DEPTH_TEST );
    glDepthFunc( GL_LEQUAL );

    glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );

    glfwSetKeyCallback( winPtr, OnKeyPressWrapper );
    glfwSetCursorPosCallback( winPtr, OnMouseMoveWrapper );

    prevTime = glfwGetTime();

    running = true;

    InitSysLog();

    return true;
}

int Test::Exec( void )
{
    if ( !winPtr )
        return 1;

    while( running && !glfwWindowShouldClose( winPtr ) )
    {
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        Run();

        glfwSwapBuffers( winPtr );
        glfwPollEvents();
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
                        camPtr->EvalKeyPress( key );
                    break;
            }
            break;

        case GLFW_RELEASE:
                camPtr->EvalKeyRelease( key );
            break;

        default:
            break;
    }
}

void Test::OnMouseMove( double x, double y )
{
    camPtr->EvalMouseMove( ( float ) x, ( float ) y );
}
