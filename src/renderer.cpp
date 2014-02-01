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

enum
{
    FACE_TYPE_BILLBOAD = 4,
    FACE_TYPE_MESH = 3,
    FACE_TYPE_PATCH = 2,
    FACE_TYPE_POLYGON = 1
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
    for ( int i = 0; i < KEY_COUNT; ++i )
    {
        keysPressed[ i ] = KEY_NOT_PRESSED;
    }
}

/*
=====================================================

Camera::~Camera

=====================================================
*/

glm::mat4 Camera::Orientation( void )
{
    /*
    glm::mat4 orient( 1.0f );

    orient = glm::rotate( orient, rotation.x, glm::vec3( 1.0f, 0.0f, 0.0f ) );
    orient = glm::rotate( orient, rotation.y, glm::vec3( 0.0f, 1.0f, 0.0f ) );
    */

    glm::quat rot = GenQuat( rotation.x, glm::vec3( 1.0f, 0.0f, 0.0f ) );
    rot *= GenQuat( rotation.y, glm::vec3( 0.0f, 1.0f, 0.0f ) );

    return glm::mat4_cast( rot );
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
    rotation.x = glm::radians( y ) * MOUSE_CAM_ROT_FACTOR;
    rotation.y = glm::radians( x ) * MOUSE_CAM_ROT_FACTOR;
}

/*
=====================================================

Camera::UpdateView

=====================================================
*/

const float CAM_STEP_SPEED = 3.0f;

void Camera::UpdateView( void )
{
    glm::vec4 moveVec( CAM_STEP_SPEED, CAM_STEP_SPEED, -CAM_STEP_SPEED, 1.0f );

    const glm::mat4& orient = Orientation();
    const glm::mat4& inverseOrient = glm::inverse( orient );

    // Each vector component is computed with one vector and then extracted individually.
    // This is because the inverseOrient transform can manipulate values in the vector
    // inadvertently for keys which haven't been pressed.

    moveVec = inverseOrient * moveVec;

    float& x = moveVec.x;
    float& y = moveVec.y;
    float& z = moveVec.z;

    x = keysPressed[ KEY_LEFT ] ? -x : keysPressed[ KEY_RIGHT ] ? x : 0.0f;
    y = keysPressed[ KEY_DOWN ] ? -y : keysPressed[ KEY_UP ] ? y : 0.0f;
    z = keysPressed[ KEY_BACKWARD ] ? -z : keysPressed[ KEY_FORWARD ] ? z : 0.0f;

    position += glm::vec3( moveVec );

    view = orient * glm::translate( glm::mat4( 1.0f ), -position );
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
    : bspProgram( 0 ),
      vao( 0 ),
      vbo( 0 ),
      lastCameraPosition( 0.0f ),
      visibleFaces( NULL )
{
}

/*
=====================================================

BSPRenderer::~BSPRenderer

=====================================================
*/

BSPRenderer::~BSPRenderer( void )
{
    if ( visibleFaces )
    {
        free( visibleFaces );
    }

    glDeleteVertexArrays( 1, &vao );
    glDeleteProgram( bspProgram );
    glDeleteBuffers( 1, &vbo );
}

/*
=====================================================

BSPRenderer::Prep

Load static, independent data which need not be re-initialized if multiple maps are created.

=====================================================
*/

void BSPRenderer::Prep( void )
{
    glGenBuffers( 1, &vbo );
    glGenVertexArrays( 1, &vao );

    GLuint shaders[] =
    {
        CompileShader( "src/main.vert", GL_VERTEX_SHADER ),
        CompileShader( "src/main.frag", GL_FRAGMENT_SHADER )
    };

    bspProgram = LinkProgram( shaders, 2 );

    glUseProgram( bspProgram );
}

/*
=====================================================

BSPRenderer::Load

    Load map file specified by param filepath.

=====================================================
*/

void BSPRenderer::Load( const std::string& filepath )
{
    if ( map.IsAllocated() )
    {
        map.DestroyMap();

        if ( visibleFaces )
        {
            free( visibleFaces );
            visibleFaces = NULL;
        }
    }

    map.Read( filepath, 4 );

    visibleFaces = ( byte* )malloc( map.numFaces );
    memset( visibleFaces, 0, map.numFaces );

    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    glBufferData( GL_ARRAY_BUFFER, sizeof( BSPVertex ) * map.numVertexes, map.vertexes, GL_STATIC_DRAW );

    glBindVertexArray( vao );

    glBindAttribLocation( bspProgram, 0, "position" );
    glBindAttribLocation( bspProgram, 1, "color" );

    glEnableVertexAttribArray( 0 );
    glEnableVertexAttribArray( 1 );
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( BSPVertex ), ( void* ) offsetof( BSPVertex, position ) );
    glVertexAttribPointer( 1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof( BSPVertex ), ( void* ) offsetof( BSPVertex, color ) );

    camera.SetPerspective( 75.0f, 16.0f / 9.0f, 0.1f, 9000.0f );
}

/*
=====================================================

BSPRenderer::Draw

=====================================================
*/

void BSPRenderer::Draw( void )
{
    glUniform4f( glGetUniformLocation( bspProgram, "color0" ), 1.0f, 0.0f, 1.0f, 0.5f );
    glUniform4f( glGetUniformLocation( bspProgram, "color1" ), 0.0f, 0.5f, 1.0f, 1.0f );
    glUniform1f( glGetUniformLocation( bspProgram, "interp" ), deltaTime * ( ( float )random() / 100.0f ) );

    for ( int i = 0; i < map.numFaces; ++i )
    {
        if ( visibleFaces[ i ] == 0 )
            continue;

        const BSPFace* const face = &map.faces[ i ];

        /*
        switch( face->type )
        {
            case FACE_TYPE_MESH:
                glDrawElements( GL_TRIANGLES, face->numMeshVertexes, GL_UNSIGNED_INT, ( void* ) &map.meshVertexes[ face->meshVertexOffset ].offset );
                glDraw
                break;

            case FACE_TYPE_POLYGON:

                break;
        }
        */

        glDrawElements( GL_TRIANGLES, face->numVertexes, GL_UNSIGNED_INT, ( void* ) &face->meshVertexOffset );
        glDrawElements( GL_TRIANGLES, face->numMeshVertexes, GL_UNSIGNED_INT, ( void* ) &map.meshVertexes[ face->meshVertexOffset ].offset );

        glUniform4f( glGetUniformLocation( bspProgram, "color0" ), 1.0f, 1.0f, 1.0f, 1.0f );
        glUniform4f( glGetUniformLocation( bspProgram, "color1" ), 0.0f, 0.0f, 0.0f, 0.0f );

        glDrawElements( GL_LINE_LOOP, face->numVertexes, GL_UNSIGNED_INT, ( void* ) &face->meshVertexOffset );
        glDrawElements( GL_LINE_LOOP, face->numMeshVertexes, GL_UNSIGNED_INT, ( void* ) &map.meshVertexes[ face->meshVertexOffset ].offset );

        ExitOnGLError( "Draw" );
    }

    //memset( visibleFaces, 0, sizeof( byte ) * map.numFaces );
}

/*
=====================================================

BSPRenderer::Update

    Update camera and visibility data, using the Quake3Map
    instance held by this class to determine which faces
    in the map are visible at this moment

=====================================================
*/

void BSPRenderer::Update( float dt )
{
    deltaTime = dt;

    BSPLeaf* currentLeaf = map.FindClosestLeaf( camera.position );

    for ( int i = 0; i < map.numLeaves; ++i )
    {
        if ( map.IsClusterVisible( currentLeaf->clusterIndex, map.leaves[ i ].clusterIndex ) )
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

