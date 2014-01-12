#include "GLWidget.h"
#include <QKeyEvent>
#include <QCoreApplication>
#include <QTimer>
#include "Quake3Map.h"

static QGLFormat makeFormat()
{
    QGLFormat glFormat;
    glFormat.setVersion( 3, 3 );
    glFormat.setProfile( QGLFormat::CoreProfile );
    glFormat.setSampleBuffers( true );

    return glFormat;
}

GLWidget::GLWidget( QWidget* parent )
    :   QGLWidget( makeFormat(), parent )
{
    this->setUpdatesEnabled( true );
    this->setAutoBufferSwap( true );
    this->setFocus();

    QTimer* timer = new QTimer( this );
    connect( timer, SIGNAL( timeout() ), this, SLOT( updateGL() ) );
    timer->start( 10 );
}

GLWidget::~GLWidget( void )
{
}

void GLWidget::initializeGL( void )
{
    glewExperimental = true;
    GLenum response = glewInit();

    if ( response != GLEW_OK )
    {
        qWarning() << "Could not initialize GLEW! " << glewGetErrorString( response );
    }

    QGLFormat glFormat = QGLWidget::format();

    if ( !glFormat.sampleBuffers() )
    {
        qWarning() << "Could not enable sample buffers";
    }

    // Set the clear color to black
    glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );

    mRenderer.allocBase();

    mRenderer.loadMap( "asset/quake/aty3dm1v2.bsp" );
    // TODO: load map here.
}

void GLWidget::resizeGL( int width, int height )
{
    glViewport( 0, 0, width, qMax( height, 1 ) );
}

void GLWidget::paintGL( void )
{
    mRenderer.render();
}

void GLWidget::updateGL( void )
{
    update();

    mRenderer.update();
}

static const float CAM_SPEED = 1.0f;

void GLWidget::keyPressEvent( QKeyEvent* e )
{

    GLCamera* const camera = &mRenderer.mCamera;

    switch ( e->key() )
    {
        case Qt::Key_Escape:
            QCoreApplication::instance()->quit();
            break;

        case Qt::Key_W:
            camera->walk( CAM_SPEED );
            break;
        case Qt::Key_S:
            camera->walk( -CAM_SPEED );
            break;
        case Qt::Key_A:
            camera->strafe( -CAM_SPEED );
            break;
        case Qt::Key_D:
            camera->strafe( CAM_SPEED );
            break;
        case Qt::Key_Space:
            camera->raise( CAM_SPEED );
            break;
        case Qt::Key_Control:
            camera->raise( -CAM_SPEED );
            break;

        default:
            QGLWidget::keyPressEvent( e );
            break;
    }
}


