#include "GLWidget.h"
#include "Quake3Map.h"

GLWidget::GLWidget(void )
{

}

GLWidget::~GLWidget( void )
{
}

void GLWidget::initializeGL( void )
{


    /*
    glewExperimental = true;
    GLenum response = glewInit();

    if ( response != GLEW_OK )
    {
        qWarning() << "Could not initialize GLEW! " << glewGetErrorString( response );
    }
    */

    // Set the clear color to black
    glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );



    //mRenderer.allocBase();

    //mRenderer.loadMap( "asset/quake/aty3dm1v2.bsp" );
    // TODO: load map here.
}

void GLWidget::resizeGL( int width, int height )
{
    glViewport( 0, 0, width, std::max( height, 1 ) );
}

void GLWidget::paintGL( void )
{
    //mRenderer.render();
}

void GLWidget::updateGL( void )
{
    mRenderer.update();
}

static const float CAM_SPEED = 1.0f;

void GLWidget::keyPressEvent( int key )
{


}


