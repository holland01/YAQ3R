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
      mVbo( 0 ),
      mMap( NULL ),
      mLastCameraPosition( 0.0f, 0.0f, 0.0f ),
      mVisibleFaces( NULL ),
      mBspProgram( 0 )
{
    mCamera.mPosition = glm::vec3( -8.000000f, -2.000000f, -214.000000f );
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

    if ( mBspProgram != 0 )
    {
        glDeleteProgram( mBspProgram );
    }
}

void GLRenderer::allocBase( void )
{
    GLuint shaders[] =
    {
        loadShader( "src/test.vert", GL_VERTEX_SHADER ),
        loadShader( "src/test.frag", GL_FRAGMENT_SHADER )
    };

    mBspProgram = makeProgram( shaders, 2 );
}

void GLRenderer::loadMap( const std::string& filepath )
{
    if ( mMap )
    {
        delete mMap;
    }

    mMap = new Quake3Map;

    mMap->read( filepath, 24 );
    initLogBaseData( mMap );

    mVisibleFaces = ( byte* )malloc( mMap->mTotalFaces );
    memset( mVisibleFaces, 0, mMap->mTotalFaces );

    glGenVertexArrays( 1, &mVao );
    glBindVertexArray( mVao );

    glGenBuffers( 1, &mVbo );
    glBindBuffer( GL_ARRAY_BUFFER, mVbo );
    glBufferData( GL_ARRAY_BUFFER, sizeof( BspVertex ) * mMap->mTotalVertexes, ( void* ) mMap->mVertexes, GL_STATIC_DRAW );

    GLint positionAttrib = glGetAttribLocation( mBspProgram, "position" );

    glEnableVertexAttribArray( positionAttrib );
    glVertexAttribPointer( positionAttrib, 3, GL_FLOAT, GL_FALSE, sizeof( BspVertex ), ( void* ) offsetof( BspVertex, BspVertex::position ) );

    glBindVertexArray( 0 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );

    mCamera.setPerspective( 45.0f, 4.0f / 3.0f, 0.1f, 100.0f );
}

static glm::mat4 _tempModel = glm::scale( glm::mat4( 1.0f ), glm::vec3( 20.0f, 20.0f, 20.0f ) );

void GLRenderer::draw( void )
{
    glBindVertexArray( mVao );
    glBindBuffer( GL_ARRAY_BUFFER, mVbo );
    glUseProgram( mBspProgram );

    glUniform4f( glGetUniformLocation( mBspProgram, "color0" ), 1.0f, 1.0f, 1.0f, 1.0f );
    glUniform4f( glGetUniformLocation( mBspProgram, "color1" ), 1.0f, 0.0f, 1.0f, 1.0f );

    for ( int i = 0; i < mMap->mTotalFaces; ++i )
    {
        if ( mVisibleFaces[ i ] == 0 || ( mMap->mFaces[ i ].type != 3 && mMap->mFaces[ i ].type != 1 ) )
            continue;

        const BspFace* const face = &mMap->mFaces[ mVisibleFaces[ i ] ];

        glUniformMatrix4fv( glGetUniformLocation( mBspProgram, "model" ), 1, GL_FALSE, glm::value_ptr( _tempModel ) );

        glDrawElements( GL_TRIANGLE_FAN, face->numMeshVertexes, GL_UNSIGNED_INT, &mMap->mMeshVertexes[ face->meshVertexOffset ].offset );

        //glDrawArrays( GL_LINE_STRIP, face->vertexOffset, face->numVertexes );

        logDrawCall( face, mMap->mMeshVertexes );
    }

    glUseProgram( 0 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
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

                        myPrintf( "Visible Faces", "Face Found!" );
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

