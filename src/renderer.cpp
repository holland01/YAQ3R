#include "renderer.h"
#include "q3m.h"
#include "shader.h"
#include "log.h"
#include "math_util.h"
#include "input.h"

enum
{
    FACE_TYPE_BILLBOAD = 4,
    FACE_TYPE_MESH = 3,
    FACE_TYPE_PATCH = 2,
    FACE_TYPE_POLYGON = 1
};

/*
=====================================================

RenderPass::RenderPass

=====================================================
*/

RenderPass::RenderPass( void )
    : position( 0.0f ),
      rotation( 0.0f ),
      view( 1.0f ),
      projection( glm::perspective( 45.0f, 16.0f / 9.0f, 0.1f, 9000.0f ) )
{
}

RenderPass::RenderPass( const RenderPass& copy )
    : position( copy.position ),
      rotation( copy.rotation ),
      view( copy.view ),
      projection( copy.projection )
{
}

RenderPass::~RenderPass( void )
{
}

/*
=====================================================

RenderPass::operator =

=====================================================
*/

RenderPass& RenderPass::operator =( RenderPass copy )
{
    position = copy.position;
    rotation = copy.rotation;
    view = copy.view;
    projection = copy.projection;

    return *this;
}

/*
=====================================================

RenderPass::Orientation

=====================================================
*/

glm::mat4 RenderPass::Orientation( void )
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

RenderPass::Reset

=====================================================
*/

void RenderPass::Reset( void )
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
        }
    }

    map.Read( filepath, 1 );

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

}

/*
=====================================================

BSPRenderer::Draw

=====================================================
*/

void BSPRenderer::Draw( const RenderPass& pass )
{
    DrawTree( 0, pass );
}

/*
=====================================================

BSPRenderer::DrawTree

=====================================================
*/

void BSPRenderer::DrawTree( int index, const RenderPass& pass )
{
    if ( index < 0 )
    {
        DrawLeafNode( -( index + 1 ), pass );
        return;
    }

    BSPNode* node = &map.nodes[ index ];
    BSPPlane* plane = &map.planes[ node->plane ];

    float d = glm::dot( pass.position, glm::vec3( plane->normal.x, plane->normal.y, plane->normal.z ) ) - plane->distance;

    if ( d >= 0 )
    {
        DrawTree( node->children[ 0 ], pass );
        DrawTree( node->children[ 1 ], pass );
    }
    else
    {
        DrawTree( node->children[ 1 ], pass );
        DrawTree( node->children[ 0 ], pass );
    }
}

void BSPRenderer::DrawLeafNode( int index, const RenderPass& pass )
{
    BSPLeaf* leaf = &map.leaves[ index ];

    for ( int i = 0; i < leaf->numLeafFaces; ++i )
    {
        int f = leaf->leafFaceOffset + i;

        if ( visibleFaces[ f ] == 1 )
        {
            BSPFace* face = map.faces + f;

            switch ( face->type )
            {
                case FACE_TYPE_MESH:
                    glDrawElements( GL_TRIANGLES, face->numMeshVertexes, GL_UNSIGNED_INT, ( void* ) &map.meshVertexes[ face->meshVertexOffset ].offset );
                    break;

                case FACE_TYPE_POLYGON:
                    glDrawElements( GL_TRIANGLE_FAN, face->numVertexes, GL_UNSIGNED_INT, ( void* ) &face->vertexOffset );
                    break;
            }

            visibleFaces[ f ] = 0;
        }
    }
}

/*
=====================================================

BSPRenderer::Update

    Update RenderPass and visibility data, using the Quake3Map
    instance held by this class to determine which faces
    in the map are visible at the time of this method's invocation

=====================================================
*/

void BSPRenderer::Update( float dt, const RenderPass& pass )
{
    deltaTime = dt;

    BSPLeaf* currentLeaf = map.FindClosestLeaf( pass.position );

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

    glUniformMatrix4fvARB( glGetUniformLocation( bspProgram, "modelview" ), 1, GL_FALSE, glm::value_ptr( pass.View() ) );
    glUniformMatrix4fvARB( glGetUniformLocation( bspProgram, "projection" ), 1, GL_FALSE, glm::value_ptr( pass.Projection() ) );
}

