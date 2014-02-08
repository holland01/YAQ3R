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

    //*/

    //return orient;
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
      deltaTime( 0 )
{
}

BSPRenderer::BSPRenderer( const BSPRenderer& copy )
    : bspProgram( copy.bspProgram ),
      vao( copy.vao ),
      vbo( copy.vbo ),
      deltaTime( copy.deltaTime ),
      visibleFaces( copy.visibleFaces ),
      transparentFaces( copy.transparentFaces ), opaqueFaces( copy.opaqueFaces )
{
}

/*
=====================================================

BSPRenderer::~BSPRenderer

=====================================================
*/

BSPRenderer::~BSPRenderer( void )
{
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

    glBindVertexArray( vao );
    glBindBuffer( GL_ARRAY_BUFFER, vbo );

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
        visibleFaces.clear();
    }

    map.Read( filepath, 1 );

    glBufferData( GL_ARRAY_BUFFER, sizeof( BSPVertex ) * map.numVertexes, map.vertexes, GL_STATIC_DRAW );

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

void BSPRenderer::Draw( void )
{
    for ( int i = 0; i < ( int ) visibleFaces.size(); ++i )
    {
        BSPFace* face = map.faces + visibleFaces[ i ];

        switch ( face->type )
        {
            case FACE_TYPE_POLYGON:
            case FACE_TYPE_MESH:
            {
                //glDrawElements( GL_TRIANGLES, face->numMeshVertexes, GL_UNSIGNED_INT, ( void* ) &map.meshVertexes[ face->meshVertexOffset ].offset );

                std::vector< int >& transMeshVerts = transparentFaces[ i ];

                if ( !transMeshVerts.empty() )
                {
                    glDrawElements( GL_TRIANGLES, transMeshVerts.size(), GL_UNSIGNED_INT, ( void* ) &transMeshVerts[ 0 ] );
                    transMeshVerts.clear();
                }

                std::vector< int >& opaqueMeshVerts = opaqueFaces[ i ];

                if ( !opaqueMeshVerts.empty() )
                {
                    glDrawElements( GL_TRIANGLES, opaqueMeshVerts.size(), GL_UNSIGNED_INT, ( void* ) &opaqueMeshVerts[ 0 ] );
                    opaqueMeshVerts.clear();
                }
            }
                break;
        }
    }

    opaqueFaces.clear();
    transparentFaces.clear();
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

    glUniformMatrix4fvARB( glGetUniformLocation( bspProgram, "modelview" ), 1, GL_FALSE, glm::value_ptr( pass.View() ) );
    glUniformMatrix4fvARB( glGetUniformLocation( bspProgram, "projection" ), 1, GL_FALSE, glm::value_ptr( pass.Projection() ) );

    visibleFaces.clear();

    BSPLeaf* currentLeaf = map.FindClosestLeaf( pass.position );

    for ( int i = 0; i < map.numLeaves; ++i )
    {
        BSPLeaf* testLeaf = map.leaves + i;

        if ( map.IsClusterVisible( currentLeaf->clusterIndex, testLeaf->clusterIndex ) )
        {
            for ( int j = testLeaf->leafFaceOffset; j < testLeaf->leafFaceOffset + testLeaf->numLeafFaces; ++j )
            {   
                visibleFaces.push_back( map.leafFaces[ j ].index );

                BSPFace* face = map.faces + map.leafFaces[ j ].index;

                std::vector< int > transparentVerts, opaqueVerts;

                for ( int k = face->meshVertexOffset; k < face->meshVertexOffset + face->numMeshVertexes; ++k )
                {
                    BSPVertex* vertex = map.vertexes + k;

                    if ( vertex->color[ 3 ] == 0 )
                    {
                        transparentVerts.push_back( map.meshVertexes[ k ].offset );
                    }
                    else
                    {
                        opaqueVerts.push_back( map.meshVertexes[ k ].offset );
                    }
                }

                transparentFaces.push_back( transparentVerts );
                opaqueFaces.push_back( opaqueVerts );
            }
        }
    }
}
