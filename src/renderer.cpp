#include "renderer.h"
#include "q3m.h"
#include "shader.h"
#include "log.h"
#include "math_util.h"

enum
{
    KEY_PRESSED = 1,
    KEY_NOT_PRESSED = 0,
    KEY_COUNT = 6,
    KEY_FORWARD = 0,
    KEY_BACKWARD = 1,
    KEY_LEFT = 2,
    KEY_RIGHT = 3,
    KEY_UP = 4,
    KEY_DOWN = 5
};

/*
=====================================================

Camera::Camera

=====================================================
*/

Camera::Camera( void )
    : position( 0.0f, 0.0f, 0.0f ),
      rotation( 0.0f, 0.0f, 0.0f ),
      mouseBoundries( 1366.0f, 768.0f ),
      view( 1.0f ),
      projection( glm::perspective( 45.0f, 16.0f / 9.0f, 0.1f, 500.0f ) )
{
    ZeroOutKeys();
}

/*
=====================================================

Camera::~Camera

=====================================================
*/

glm::mat4 Camera::Orientation( void )
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

void Camera::EvalKeyPress( int key )
{
    switch( key )
    {
        case GLFW_KEY_W:
            keysPressed[ KEY_FORWARD ] = KEY_PRESSED;
            break;

        case GLFW_KEY_S:
            keysPressed[ KEY_BACKWARD ] = KEY_PRESSED;
            break;

        case GLFW_KEY_A:
            keysPressed[ KEY_LEFT ] = KEY_PRESSED;
            break;

        case GLFW_KEY_D:
            keysPressed[ KEY_RIGHT ] = KEY_PRESSED;
            break;

        case GLFW_KEY_LEFT_SHIFT:
            keysPressed[ KEY_DOWN ] = KEY_PRESSED;
            break;

        case GLFW_KEY_SPACE:
            keysPressed[ KEY_UP ] = KEY_PRESSED;
            break;
    }
}

void Camera::EvalKeyRelease( int key )
{
    switch( key )
    {
        case GLFW_KEY_W:
            keysPressed[ KEY_FORWARD ] = KEY_NOT_PRESSED;
            break;
        case GLFW_KEY_S:
            keysPressed[ KEY_BACKWARD ] = KEY_NOT_PRESSED;
            break;
        case GLFW_KEY_A:
            keysPressed[ KEY_LEFT ] = KEY_NOT_PRESSED;
            break;
        case GLFW_KEY_D:
            keysPressed[ KEY_RIGHT ] = KEY_NOT_PRESSED;
            break;
        case GLFW_KEY_LEFT_SHIFT:
            keysPressed[ KEY_DOWN ] = KEY_NOT_PRESSED;
            break;
        case GLFW_KEY_SPACE:
            keysPressed[ KEY_UP ] = KEY_NOT_PRESSED;
            break;
    }
}

/*
=====================================================

Camera::EvalMouseMove

=====================================================
*/

const float MOUSE_CAM_ROT_FACTOR = 0.1f;

const float MOUSE_THRESHOLD = 2.0f * glm::pi< float >();
const float MOUSE_BOUNDS = glm::pi< float >();

void Camera::EvalMouseMove( float x, float y )
{
    rotation.x = -glm::cos( glm::radians( y ) );
    rotation.y = glm::sin( glm::radians( x ) );

    if ( rotation.x > MOUSE_BOUNDS )
    {
        rotation.x -= MOUSE_THRESHOLD;
    }

    if ( rotation.x < -MOUSE_BOUNDS )
    {
        rotation.x += MOUSE_THRESHOLD;
    }

    if ( rotation.y > MOUSE_BOUNDS )
    {
        rotation.y -= MOUSE_THRESHOLD;
    }

    if ( rotation.y < -MOUSE_BOUNDS )
    {
        rotation.y += MOUSE_THRESHOLD;

    }
}

/*
=====================================================

Camera::UpdateView

=====================================================
*/

const float CAM_STEP_SPEED = 1.0f;

void Camera::UpdateView( void )
{
    glm::vec4 moveVec( 0.0f, 0.0f, 0.0f, 0.0f );

    const glm::mat4& orient = Orientation();
    const glm::mat4& inverseOrient = glm::inverse( orient );

    if ( keysPressed[ KEY_FORWARD ] == KEY_PRESSED )
    {
        moveVec.z += ( inverseOrient * glm::vec4( 0.0f, 0.0f, -CAM_STEP_SPEED, 1.0f ) ).z;
    }
    else if ( keysPressed[ KEY_BACKWARD ] == KEY_PRESSED )
    {
        moveVec.z += ( inverseOrient * glm::vec4( 0.0f, 0.0f, CAM_STEP_SPEED, 1.0f ) ).z;
    }

    if ( keysPressed[ KEY_RIGHT ] == KEY_PRESSED )
    {
        moveVec.x += ( inverseOrient * glm::vec4( CAM_STEP_SPEED, 0.0f, 0.0f, 1.0f ) ).x;
    }
    else if ( keysPressed[ KEY_LEFT ] == KEY_PRESSED )
    {
        moveVec.x += ( inverseOrient * glm::vec4( -CAM_STEP_SPEED, 0.0f, 0.0f, 1.0f ) ).x;
    }

    if ( keysPressed[ KEY_UP ] == KEY_PRESSED )
    {
        moveVec.y += ( inverseOrient * glm::vec4( 0.0f, CAM_STEP_SPEED, 0.0f, 1.0f ) ).y;
    }
    else if ( keysPressed[ KEY_DOWN ] == KEY_PRESSED )
    {
        moveVec.y += ( inverseOrient * glm::vec4( 0.0f, -CAM_STEP_SPEED, 0.0f, 1.0f ) ).y;
    }

    position += glm::vec3( moveVec );

    view = orient * glm::translate( glm::mat4( 1.0f ), -position );
}

/*
=====================================================

Camera::ZeroOutKeys

Set all bools in the 'keysPressed' array to false

=====================================================
*/

void Camera::ZeroOutKeys( void )
{
    for ( int i = 0; i < KEY_COUNT; ++i )
    {
        keysPressed[ i ] = KEY_NOT_PRESSED;
    }
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

Load static, independent data which need not be re-initialized if multiple maps are created.

=====================================================
*/

void BSPRenderer::Prep( void )
{
    GLuint shaders[] =
    {
        CompileShader( "src/main.vert", GL_VERTEX_SHADER ),
        CompileShader( "src/main.frag", GL_FRAGMENT_SHADER )
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

    map->Read( filepath, 1 );

    visibleFaces = ( byte* )malloc( map->numFaces );
    memset( visibleFaces, 0, map->numFaces );

    glGenBuffers( 1, bufferObjects );
    glBindBuffer( GL_ARRAY_BUFFER, bufferObjects[ 0 ] );
    glBufferData( GL_ARRAY_BUFFER, sizeof( BSPVertex ) * map->numVertexes, map->vertexes, GL_STATIC_DRAW );

    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );

    glBindAttribLocation( bspProgram, 0, "position" );
    glBindAttribLocation( bspProgram, 1, "color" );

    glEnableVertexAttribArray( 0 );
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( BSPVertex ), ( void* )offsetof( BSPVertex, position ) );
    glVertexAttribPointer( 1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof( BSPVertex ), ( void* )offsetof( BSPVertex, color ) );

    //glBindBuffer( GL_ARRAY_BUFFER, 0 );
    //glBindVertexArray( 0 );

    camera.SetPerspective( 75.0f, 16.0f / 9.0f, 0.1f, 9000.0f );

    glUseProgram( bspProgram );
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


    for ( int i = 0; i < map->numFaces; ++i )
    {
        if ( visibleFaces[ i ] == 0 || ( map->faces[ i ].type != 3 && map->faces[ i ].type != 1 ) )
            continue;

        const BSPFace* const face = &map->faces[ i ];

        glDrawElements( GL_TRIANGLES, face->numMeshVertexes, GL_UNSIGNED_INT, ( void* ) &map->meshVertexes[ face->meshVertexOffset ].offset );
        ExitOnGLError( "Draw" );
    }
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

    glUniformMatrix4fvARB( glGetUniformLocation( bspProgram, "modelview" ), 1, GL_FALSE, glm::value_ptr( camera.View() ) );
    glUniformMatrix4fvARB( glGetUniformLocation( bspProgram, "projection" ), 1, GL_FALSE, glm::value_ptr( camera.Projection() ) );

    if ( lastCameraPosition != camera.position )
    {
        MyPrintf( "Camera Position", "x => %f, y => %f z => %f", camera.position.x, camera.position.y, camera.position.z );
    }

    lastCameraPosition = camera.position;
}

