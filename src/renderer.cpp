#include "renderer.h"
#include "q3m.h"
#include "shader.h"
#include "log.h"
#include "math_util.h"

/*
=====================================================

Camera::Camera

=====================================================
*/

Camera::Camera( void )
    : position( 0.0f, 0.0f, 0.0f ),
      rotation( 0.0f, 0.0f, 0.0f ),
      view( 1.0f ),
      projection( glm::perspective( 45.0f, 16.0f / 9.0f, 0.1f, 100.0f ) ),
      lastMouse( 0.0f, 0.0f )
{
}

/*
=====================================================

Camera::~Camera

=====================================================
*/

glm::mat4 Camera::orientation( void )
{
    glm::mat4 orient( 1.0f );

    orient = glm::rotate( orient, rotation.x, glm::vec3( 1.0f, 0.0f, 0.0f ) );
    orient = glm::rotate( orient, rotation.y, glm::vec3( 0.0f, 1.0f, 0.0f ) );

    return orient;
}

/*
=====================================================

Camera::EvalKeyPress

=====================================================
*/

const float CAM_STEP_SPEED = 1.0f;

void Camera::EvalKeyPress( int key )
{
    if ( key == GLFW_KEY_W )
    {
        Walk( CAM_STEP_SPEED );
    }

    if ( key == GLFW_KEY_S )
    {
        Walk( -CAM_STEP_SPEED );
    }

    if ( key == GLFW_KEY_A )
    {
        Strafe( -CAM_STEP_SPEED );
    }

    if ( key == GLFW_KEY_D )
    {
        Strafe( CAM_STEP_SPEED );
    }

    if ( key == GLFW_KEY_SPACE )
    {
        Raise( CAM_STEP_SPEED );
    }

    if ( key == GLFW_KEY_C )
    {
        Raise( -CAM_STEP_SPEED );
    }
}

/*
=====================================================

Camera::EvalMouseMove

=====================================================
*/

const float MOUSE_CAM_ROT_FACTOR = 0.01f;

void Camera::EvalMouseMove( float x, float y )
{
    rotation.x += -( lastMouse.y - y ) * MOUSE_CAM_ROT_FACTOR;
    rotation.y += ( lastMouse.x - x ) * MOUSE_CAM_ROT_FACTOR;

    lastMouse.x = x;
    lastMouse.y = y;
}

/*
=====================================================

Camera::Walk

=====================================================
*/

void Camera::Walk( float step )
{
    glm::vec4 forward = glm::inverse( orientation() ) * glm::vec4( 0.0f, 0.0f, -step, 1.0f );

    position += glm::vec3( forward );
}

/*
=====================================================

Camera::Strafe

=====================================================
*/

void Camera::Strafe( float step )
{
    glm::vec4 right = glm::inverse( orientation() ) * glm::vec4( step, 0.0f, 0.0f, 1.0f );

    position += glm::vec3( right );
}

/*
=====================================================

Camera::Raise

=====================================================
*/

void Camera::Raise( float step )
{
    glm::vec4 up = glm::inverse( orientation() ) * glm::vec4( 0.0f, step, 0.0f, 1.0f );

    position += glm::vec3( up );
}

/*
=====================================================

Camera::UpdateView

=====================================================
*/

void Camera::UpdateView( void )
{
    view = orientation() * glm::translate( glm::mat4( 1.0f ), -position );
}

/*
=====================================================

Camera::SetPerspective

=====================================================
*/

void Camera::SetPerspective( float fovy, float aspect, float zNear, float zFar )
{
    projection = glm::perspective( fovy, aspect, zNear, zFar );
}

/*
=====================================================

Camera::Reset

=====================================================
*/

void Camera::Reset( void )
{
    position = glm::vec3( 0.0f, 0.0f, 0.0f );
    rotation = glm::vec3( 0.0f, 0.0f, 0.0f );

    view = glm::mat4( 1.0f );
}

/*
=====================================================

BSPRenderer::BSPRenderer

=====================================================
*/

BSPRenderer::BSPRenderer( void )
    : vao( 0 ),
      map( NULL ),
      lastCameraPosition( 0.0f, 0.0f, 0.0f ),
      visibleFaces( NULL ),
      bspProgram( 0 ),
      currentLeaf( NULL )
{
}

/*
=====================================================

BSPRenderer::~BSPRenderer

=====================================================
*/

BSPRenderer::~BSPRenderer( void )
{
    if ( map )
    {
        glDeleteVertexArrays( 1, &vao );

        delete map;

        if ( visibleFaces )
        {
            free( visibleFaces );
        }
    }

    if ( bspProgram != 0 )
    {
        glDeleteProgram( bspProgram );
    }
}

/*
=====================================================

BSPRenderer::Prep
info_log
Load static, independent data which need not be re-initialized if multiple maps are created.

=====================================================
*/

void BSPRenderer::Prep( void )
{
    GLuint shaders[] =
    {
        CompileShader( "src/test.vert", GL_VERTEX_SHADER ),
        CompileShader( "src/test.frag", GL_FRAGMENT_SHADER )
    };

    bspProgram = LinkProgram( shaders, 2 );
}

/*
=====================================================

BSPRenderer::Load

    Load map file specified by param filepath.

=====================================================
*/

void BSPRenderer::Load( const std::string& filepath )
{
    if ( map )
    {
        delete map;
    }

    map = new Quake3Map;

    map->Read( filepath, 3 );

    visibleFaces = ( byte* )malloc( map->numFaces );
    memset( visibleFaces, 0, map->numFaces );

    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );

    glGenBuffers( 1, bufferObjects );
    glBindBuffer( GL_ARRAY_BUFFER, bufferObjects[ 0 ] );
    glBufferData( GL_ARRAY_BUFFER, sizeof( BSPVertex ) * map->numVertexes, map->vertexes, GL_STATIC_DRAW );

    GLint positionAttrib = glGetAttribLocation( bspProgram, "position" );

    glEnableVertexAttribArray( positionAttrib );
    glVertexAttribPointer( positionAttrib, 3, GL_FLOAT, GL_FALSE, 0, ( void* )0 );

    glBindVertexArray( 0 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );

    camera.SetPerspective( 75.0f, 16.0f / 9.0f, 0.1f, 800.0f );
}

/*
=====================================================

BSPRenderer::Draw

=====================================================
*/

void BSPRenderer::Draw( void )
{
    if ( !currentLeaf )
    {
        return;
    }

    glBindVertexArray( vao );
    glBindBuffer( GL_ARRAY_BUFFER, bufferObjects[ 0 ] );
    glUseProgram( bspProgram );

    for ( int i = 0; i < map->numFaces; ++i )
    {
        if ( visibleFaces[ i ] == 0 || ( map->faces[ i ].type != 3 && map->faces[ i ].type != 1 ) )
            continue;

        const BSPFace* const face = &map->faces[ i ];


        glDrawElements( GL_TRIANGLES, face->numMeshVertexes, GL_UNSIGNED_INT, ( void* ) &map->meshVertexes[ face->meshVertexOffset ].offset );
       // LogDrawCall( i, boundsCenter, camera.position, faceTransform, face, map );
    }

    // Zero this out to reset visibility on the next update.
   // memset( visibleFaces, 0, sizeof( byte ) * map->numFaces );

    glUseProgram( 0 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindVertexArray( 0 );
}

/*
=====================================================

BSPRenderer::Update

    Update camera and visibility data, using the Quake3Map
    instance held by this class to determine which faces
    in the map are visible at this moment

=====================================================
*/

void BSPRenderer::Update( void )
{

    currentLeaf = map->FindClosestLeaf( camera.position );

    for ( int i = 0; i < map->numLeaves; ++i )
    {
        if ( map->IsClusterVisible( currentLeaf->clusterIndex, map->leaves[ i ].clusterIndex ) )
        {
            for ( int f = currentLeaf->leafFaceOffset; f < currentLeaf->leafFaceOffset + currentLeaf->numLeafFaces; ++f )
            {
                if ( visibleFaces[ f ] == 0 )
                {
                    visibleFaces[ f ] = 1;

                    MyPrintf( "Visible Faces", "Face Found!" );
                }
            }
        }
    }

    camera.UpdateView();

    glUniformMatrix4fv( glGetUniformLocation( bspProgram, "modelview" ), 1, GL_FALSE, glm::value_ptr( camera.View() ) );
    glUniformMatrix4fv( glGetUniformLocation( bspProgram, "projection" ), 1, GL_FALSE, glm::value_ptr( camera.Projection() ) );

    if ( lastCameraPosition != camera.position )
    {
        MyPrintf( "Camera Position", "x => %f, y => %f z => %f", camera.position.x, camera.position.y, camera.position.z );
    }

    lastCameraPosition = camera.position;


}

