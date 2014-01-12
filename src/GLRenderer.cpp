#include "GLRenderer.h"
#include "Quake3Map.h"


/*
=====================================================

                    GLCamera

=====================================================
*/


GLCamera::GLCamera( void )
    : mRotation( 0.0f, 0.0f, 0.0f ),
      mPosition( 0.0f, 0.0f, 0.0f )
{
    mView.setToIdentity();
    mProjection.setToIdentity();
}

QMatrix4x4 GLCamera::orientation( void )
{
    QMatrix4x4 orient;
    orient.setToIdentity();

    orient.rotate( mRotation.x(), QVector3D( 1.0f, 0.0f, 0.0f ) );
    orient.rotate( mRotation.y(), QVector3D( 0.0f, 1.0f, 0.0f ) );

    return orient;
}

void GLCamera::walk( float step )
{
    QVector4D forward = orientation().inverted() * QVector4D( 0.0f, 0.0f, -step, 1.0f );

    mPosition += forward.toVector3D();
}

void GLCamera::strafe( float step )
{
    QVector4D right = orientation().inverted() * QVector4D( step, 0.0f, 0.0f, 1.0f );

    mPosition += right.toVector3D();
}

void GLCamera::raise( float step )
{
    QVector4D up = orientation().inverted() * QVector4D( 0.0f, step, 0.0f, 1.0f );

    mPosition += up.toVector3D();
}

void GLCamera::rotateX( float angDeg )
{
    mRotation.setX( angDeg );
}

void GLCamera::rotateY( float angDeg )
{
    mRotation.setY( angDeg );
}

void GLCamera::updateView( void )
{
    mView = orientation();
    mView.translate( -mPosition );
}

void GLCamera::setPerspective( float fovy, float aspect, float zNear, float zFar )
{
    mProjection.perspective( fovy, aspect, zNear, zFar );
}

void GLCamera::reset( void )
{
    mPosition = QVector3D( 0.0f, 0.0f, 0.0f );
    mRotation = QVector3D( 0.0f, 0.0f, 0.0f );

    mView.setToIdentity();
}

/*
=====================================================

                    GLRenderer

=====================================================
*/

GLRenderer::GLRenderer( void )
    : mVao( 0 ),
      mMapData( NULL ),
      mLastCameraPosition( 0.0f, 0.0f, 0.0f )
{
}

GLRenderer::~GLRenderer( void )
{
    if ( mMapData )
    {
        glDeleteVertexArrays( 1, &mVao );

        delete mMapData;
    }
}

void GLRenderer::allocBase( void )
{
    linkProgram( "src/test.vert", "src/test.frag" );
    mProjectionUnif = mProgram.uniformLocation( "projection" );
    mModelViewUnif = mProgram.uniformLocation( "modelView" );
}

void GLRenderer::loadMap( const std::string& filepath )
{
    if ( mMapData )
    {
        delete mMapData;
    }

    mMapData = new Quake3Map;

    mMapData->read( filepath );

    mMapData->loadVertexBuffer( mVertexBuffer );
    mMapData->loadIndexBuffer( mIndexBuffer );

    mVertexBuffer.bind();
    mProgram.bind();

    glGenVertexArrays( 1, &mVao );
    glBindVertexArray( mVao );

    mProgram.enableAttributeArray( 0 );
    mProgram.setAttributeBuffer( 0, GL_FLOAT, offsetof( BspVertex, BspVertex::position ), 3, sizeof( BspVertex ) );

    mCamera.setPerspective( 45.0f, 4.0f / 3.0f, 0.1f, 100.0f );
}

void GLRenderer::render( void )
{
    auto visibleFacesEnd = mVisibleFaces.end();

    mIndexBuffer.bind();

    for ( auto f = mVisibleFaces.begin(); f != visibleFacesEnd; ++f )
    {
        const BspFace* const face = mMapData->mFaceBuffer + *f;

        glDrawElements( GL_TRIANGLES, face->numMeshVertices, GL_UNSIGNED_INT, &mMapData->mMeshVertices[ face->meshVertexOffset ] );
    }

    mIndexBuffer.release();
}

void GLRenderer::update( void )
{
    if ( mCamera.position() != mLastCameraPosition )
    {
        BspLeaf* leaf = mMapData->findClosestLeaf( mCamera.position() );

        for ( int i = 0; i < mMapData->mVisdata->length; ++i )
        {
            if ( mMapData->isClusterVisible( leaf->clusterIndex, i ) )
            {
                int faceEnd = leaf->leafFaceOffset + leaf->numLeafFaces;

                for ( int f = leaf->leafFaceOffset; f < faceEnd; ++f )
                {
                    if ( mVisibleFaces.find( f ) == mVisibleFaces.end() )
                    {
                        mVisibleFaces.insert( f );
                    }
                }
            }
        }

        mCamera.updateView();

        mProgram.setUniformValue( mModelViewUnif, mCamera.view() );
        mProgram.setUniformValue( mProjectionUnif, mCamera.projection() );

        mLastCameraPosition = mCamera.position();
    }
}

bool GLRenderer::linkProgram( const QString& vertexShaderPath, const QString& fragShaderPath )
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
