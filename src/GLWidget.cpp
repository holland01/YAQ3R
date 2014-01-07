#include "GLWidget.h"
#include <QKeyEvent>
#include <QCoreApplication>
#include <QTimer>
#include "Q3Map.h"

static QGLFormat makeFormat()
{
    QGLFormat glFormat;
    glFormat.setVersion( 3, 3 );
    glFormat.setProfile( QGLFormat::CoreProfile );
    glFormat.setSampleBuffers( true );

    return glFormat;
}

GLWidget::GLWidget( QWidget* parent )
    :   QGLWidget( makeFormat(), parent ),
        mVertexBuffer( QGLBuffer::VertexBuffer )
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
    mVertexBuffer.destroy();
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

    // Prepare a complete shader program...
    if ( !linkProgram( ":/shaders/test.vert", ":/shaders/test.frag" ) )
        return;

    // We need us some vertex data. Start simple with a triangle ;-)
    float points[] = { -0.5f, -0.5f, 0.0f,
                        0.5f, -0.5f, 0.0f,
                        0.0f,  0.5f, 0.0f };

    mModel.translate( 0.0f, 0.0f, 1.0f );
    mModel.scale( QVector3D( 5.0f, 5.0f, 5.0f ) );

    mVertexBuffer.create();
    mVertexBuffer.setUsagePattern( QGLBuffer::StaticDraw );

    if ( !mVertexBuffer.bind() )
    {
        qWarning() << "Could not bind vertex buffer to the context";
        return;
    }

    mVertexBuffer.allocate( points, 3 * 3 * sizeof( float ) );

    // Bind the shader program so that we can associate variables from
    // our application to the shaders
    if ( !mProgram.bind() )
    {
        qWarning() << "Could not bind shader program to context";
        return;
    }

    // Enable the "vertex" attribute to bind it to our currently bound
    // vertex buffer.

    glGenVertexArrays( 1, &mVao );
    glBindVertexArray( mVao );

    mProgram.setAttributeBuffer( 0, GL_FLOAT, 0, 3 );
    mProgram.enableAttributeArray( 0 );

    mCamera.setPerspective( 45.0f, 4.0f / 3.0f, 0.1f, 100.0f );

    Quake3Map map;

    map.read( "asset/quake/aty3dm1v2.bsp"  );
}

void GLWidget::resizeGL( int width, int height )
{
    glViewport( 0, 0, width, qMax( height, 1 ) );
}

void GLWidget::paintGL( void )
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    mProgram.setUniformValue( mProgram.uniformLocation( "modelView" ), mCamera.view() * mModel );
    mProgram.setUniformValue( mProgram.uniformLocation( "projection" ), mCamera.projection() );

    glDrawArrays( GL_TRIANGLES, 0, 3 );
}

void GLWidget::updateGL( void )
{
    update();

    mCamera.updateView();
}

static const float CAM_SPEED = 1.0f;

void GLWidget::keyPressEvent( QKeyEvent* e )
{

    switch ( e->key() )
    {
        case Qt::Key_Escape:
            QCoreApplication::instance()->quit();
            break;

        case Qt::Key_W:
            mCamera.walk( CAM_SPEED );
            break;
        case Qt::Key_S:
            mCamera.walk( -CAM_SPEED );
            break;
        case Qt::Key_A:
            mCamera.strafe( -CAM_SPEED );
            break;
        case Qt::Key_D:
            mCamera.strafe( CAM_SPEED );
            break;
        case Qt::Key_Space:
            mCamera.raise( CAM_SPEED );
            break;
        case Qt::Key_Control:
            mCamera.raise( -CAM_SPEED );
            break;

        default:
            QGLWidget::keyPressEvent( e );
            break;
    }
}

bool GLWidget::linkProgram( const QString& vertexShaderPath, const QString& fragShaderPath )
{
    bool result = mProgram.addShaderFromSourceFile( QGLShader::Vertex, vertexShaderPath );
    if ( !result )
        qWarning() << mProgram.log();

    result = mProgram.addShaderFromSourceFile( QGLShader::Fragment, fragShaderPath );
    if ( !result )
        qWarning() << mProgram.log();

    result = mProgram.link();
    if ( !result )
        qWarning() << "Could not link shader program:" << mProgram.log();

    return result;
}
