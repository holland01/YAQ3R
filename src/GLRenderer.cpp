#include "GLRenderer.h"
#include "Quake3Map.h"


/*
=====================================================

                    GLCamera

=====================================================
*/


GLCamera::GLCamera( void )
    : mPosition( 0.0f, 0.0f, 0.0f ),
      mRotation( 0.0f, 0.0f, 0.0f ),
      mView( 1.0f ),
      mProjection( 1.0f )
{
}

glm::mat4 GLCamera::orientation( void )
{
    glm::mat4 orient( 1.0f );

    orient = glm::rotate( orient, mRotation.x, glm::vec3( 1.0f, 0.0f, 0.0f ) );
    orient = glm::rotate( orient, mRotation.y, glm::vec3( 0.0f, 1.0f, 0.0f ) );

    return orient;
}

void GLCamera::walk( float step )
{
    glm::vec4 forward = glm::inverse( orientation() ) * glm::vec4( 0.0f, 0.0f, -step, 1.0f );

    mPosition += glm::vec3( forward );
}

void GLCamera::strafe( float step )
{
    glm::vec4 right = glm::inverse( orientation() ) * glm::vec4( step, 0.0f, 0.0f, 1.0f );

    mPosition += glm::vec3( right );
}

void GLCamera::raise( float step )
{
    glm::vec4 up = glm::inverse( orientation() ) * glm::vec4( 0.0f, step, 0.0f, 1.0f );

    mPosition += glm::vec3( up );
}

void GLCamera::updateView( void )
{
    mView = orientation() * glm::translate( glm::mat4( 1.0f ), -mPosition );
}

void GLCamera::setPerspective( float fovy, float aspect, float zNear, float zFar )
{
    mProjection = glm::perspective( fovy, aspect, zNear, zFar );
}

void GLCamera::reset( void )
{
    mPosition = glm::vec3( 0.0f, 0.0f, 0.0f );
    mRotation = glm::vec3( 0.0f, 0.0f, 0.0f );

    mView = glm::mat4( 1.0f );
}

/*
=====================================================

                    GLRenderer

=====================================================
*/

GLRenderer::GLRenderer( void )
    : mVao( 0 ),
      mMap( NULL ),
      mLastCameraPosition( 0.0f, 0.0f, 0.0f ),
      mVisibleFaces( NULL )
{
}

GLRenderer::~GLRenderer( void )
{
    if ( mMap )
    {
        glDeleteVertexArrays( 1, &mVao );

        delete mMap;
    }

    if ( mVisibleFaces )
    {
        delete mVisibleFaces;
    }
}

void GLRenderer::allocBase( void )
{

    //mProjectionUnif = mProgram.uniformLocation( "projection" );
    //mModelViewUnif = mProgram.uniformLocation( "modelView" );
}

void GLRenderer::loadMap( const std::string& filepath )
{
    if ( mMap )
    {
        delete mMap;
    }

    mMap = new Quake3Map;

    mMap->read( filepath, 16 );

    mVisibleFaces = ( byte* )malloc( mMap->mTotalFaces );
    memset( mVisibleFaces, 0, mMap->mTotalFaces );

    glGenVertexArrays( 1, &mVao );
    glBindVertexArray( mVao );

    glBindVertexArray( 0 );

    mCamera.setPerspective( 45.0f, 4.0f / 3.0f, 0.1f, 100.0f );
}

void GLRenderer::render( void )
{
    glBindVertexArray( mVao );

    glBindVertexArray( 0 );
}

void GLRenderer::update( void )
{
    if ( mCamera.mPosition != mLastCameraPosition )
    {
        BspLeaf* leaf = mMap->findClosestLeaf( mCamera.mPosition );

        for ( int i = 0; i < mMap->mVisData->numVectors; ++i )
        {
            if ( mMap->isClusterVisible( leaf->clusterIndex, i ) )
            {
                for ( int f = leaf->leafFaceOffset; f < leaf->leafFaceOffset + leaf->numLeafFaces; ++f )
                {
                    if ( mVisibleFaces[ f ] == 0 )
                    {
                        mVisibleFaces[ f ] = 1;
                    }
                }
            }
        }

        mCamera.updateView();


        // TODO: update view and projection matrices
        //mProgram.setUniformValue( mModelViewUnif, mCamera.view() );
        //mProgram.setUniformValue( mProjectionUnif, mCamera.projection() );

        mLastCameraPosition = mCamera.mPosition;
    }
}

