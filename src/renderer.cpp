#include "renderer.h"
#include "q3m.h"
#include "shader.h"
#include "log.h"

/*
=====================================================

                    GLCamera

=====================================================
*/


GLCamera::GLCamera( void )
    : mPosition( 0.0f, 0.0f, 0.0f ),
      mRotation( 0.0f, 0.0f, 0.0f ),
      mView( 1.0f ),
      mProjection( 1.0f ),
      mLastMouse( 0.0f, 0.0f )
{
}

glm::mat4 GLCamera::orientation( void )
{
    glm::mat4 orient( 1.0f );

    orient = glm::rotate( orient, mRotation.x, glm::vec3( 1.0f, 0.0f, 0.0f ) );
    orient = glm::rotate( orient, mRotation.y, glm::vec3( 0.0f, 1.0f, 0.0f ) );

    return orient;
}

const float CAM_STEP_SPEED = 2.0f;

void GLCamera::evalKeyPress( int key )
{
    if ( key == GLFW_KEY_W )
    {
        walk( CAM_STEP_SPEED );
    }

    if ( key == GLFW_KEY_S )
    {
        walk( -CAM_STEP_SPEED );
    }

    if ( key == GLFW_KEY_A )
    {
        strafe( -CAM_STEP_SPEED );
    }

    if ( key == GLFW_KEY_D )
    {
        strafe( CAM_STEP_SPEED );
    }

    if ( key == GLFW_KEY_SPACE )
    {
        raise( CAM_STEP_SPEED );
    }

    if ( key == GLFW_KEY_C )
    {
        raise( -CAM_STEP_SPEED );
    }
}

const float MOUSE_CAM_ROT_FACTOR = 0.01f;

void GLCamera::evalMouseCoords( float x, float y )
{
    mRotation.x += -( mLastMouse.y - y ) * MOUSE_CAM_ROT_FACTOR;
    mRotation.y += ( mLastMouse.x - x ) * MOUSE_CAM_ROT_FACTOR;

    mLastMouse.x = x;
    mLastMouse.y = y;
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

BSPRenderer::BSPRenderer( void )
    : mVao( 0 ),
      mMap( NULL ),
      mLastCameraPosition( 0.0f, 0.0f, 0.0f ),
      mVisibleFaces( NULL ),
      mBspProgram( 0 )
{
}

BSPRenderer::~BSPRenderer( void )
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

    if ( mBspProgram != 0 )
    {
        glDeleteProgram( mBspProgram );
    }
}

void BSPRenderer::allocBase( void )
{
    GLuint shaders[] =
    {
        loadShader( "src/test.vert", GL_VERTEX_SHADER ),
        loadShader( "src/test.frag", GL_FRAGMENT_SHADER )
    };

    mBspProgram = makeProgram( shaders, 2 );
}

void BSPRenderer::loadMap( const std::string& filepath )
{
    if ( mMap )
    {
        delete mMap;
    }

    mMap = new Quake3Map;

    mMap->read( filepath, 1 );
    initLogBaseData( mMap );

    mVisibleFaces = ( byte* )malloc( mMap->mTotalFaces );
    memset( mVisibleFaces, 0, mMap->mTotalFaces );

    glGenVertexArrays( 1, &mVao );
    glBindVertexArray( mVao );

    glGenBuffers( 1, mBuffers );
    glBindBuffer( GL_ARRAY_BUFFER, mBuffers[ 0 ] );
    glBufferData( GL_ARRAY_BUFFER, sizeof( BspVertex ) * mMap->mTotalVertexes, mMap->mVertexes, GL_STATIC_DRAW );

    GLint positionAttrib = glGetAttribLocation( mBspProgram, "position" );

    glEnableVertexAttribArray( positionAttrib );
    glVertexAttribPointer( positionAttrib, 3, GL_FLOAT, GL_FALSE, 0, ( void* )0 );

    glBindVertexArray( 0 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );

    mCamera.setPerspective( 75.0f, 16.0f / 9.0f, 0.1f, 800.0f );
}

static glm::mat4 testModel( 1.0f );
const glm::mat4& rotMatrix = glm::rotate( glm::mat4( 1.0f ), glm::radians( 1.0f ), glm::vec3( 1.0f, 1.0f, 0.0f ) );

void BSPRenderer::draw( void )
{
    glBindVertexArray( mVao );
    glBindBuffer( GL_ARRAY_BUFFER, mBuffers[ 0 ] );
    glUseProgram( mBspProgram );

    for ( int i = 0; i < mMap->mTotalFaces; ++i )
    {
        if ( mVisibleFaces[ i ] == 0 || ( mMap->mFaces[ i ].type != 3 && mMap->mFaces[ i ].type != 1 ) )
            continue;

        const BspFace* const face = &mMap->mFaces[ i ];

        glUniformMatrix4fv( glGetUniformLocation( mBspProgram, "model" ), 1, GL_FALSE, glm::value_ptr( testModel ) );

        glDrawArrays( GL_TRIANGLES, face->meshVertexOffset, face->numMeshVertexes );
        logDrawCall( i, mCamera.mPosition, face, mMap );
    }

    // Zero this out to reset visibility on the next update.
    memset( mVisibleFaces, 0, sizeof( byte ) * mMap->mTotalFaces );

    glUseProgram( 0 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindVertexArray( 0 );

    testModel *= rotMatrix;
}

static int debug_visibleFaceCount = 0;

void BSPRenderer::update( void )
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

                        myPrintf( "Visible Faces", "Face Found! Num visible faces: %i", ++debug_visibleFaceCount );
                    }
                }
            }
        }

        mCamera.updateView();

        glUniformMatrix4fv( glGetUniformLocation( mBspProgram, "view" ), 1, GL_FALSE, glm::value_ptr( mCamera.view() ) );
        glUniformMatrix4fv( glGetUniformLocation( mBspProgram, "projection" ), 1, GL_FALSE, glm::value_ptr( mCamera.projection() ) );

        mLastCameraPosition = mCamera.mPosition;

        myPrintf( "Camera Position", "x => %f, y => %f z => %f", mCamera.mPosition.x, mCamera.mPosition.y, mCamera.mPosition.z );
    }
}

