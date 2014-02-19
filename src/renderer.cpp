#include "renderer.h"
#include "shader.h"
#include "log.h"
#include "math_util.h"

using namespace std;

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

RenderPass::RenderPass( const Q3BspParser* const map, const ViewParams& viewData )
    : view( viewData ),
      facesRendered( map->numFaces )
{
    facesRendered.assign( map->numFaces, false );
}

/*
=====================================================

RenderPass::~RenderPass

=====================================================
*/

RenderPass::~RenderPass( void )
{
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
    glGenVertexArrays( 1, &vao );
    glGenBuffers( 1, &vbo );

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

void BSPRenderer::Load( const string& filepath )
{
    if ( map.IsAllocated() )
    {
        map.DestroyMap();
    }

    map.Read( filepath );
    map.GenTextures( filepath );
    visibleFaces.reserve( map.numFaces );

    glBindVertexArray( vao );

    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    glBufferData( GL_ARRAY_BUFFER, sizeof( bspVertex_t ) * map.numVertexes, map.vertexes, GL_STATIC_DRAW );

    glBindAttribLocation( bspProgram, 0, "position" );
    glBindAttribLocation( bspProgram, 1, "color" );
    glBindAttribLocation( bspProgram, 2, "tex0" );

    glEnableVertexAttribArray( 0 );
    glEnableVertexAttribArray( 1 );
    glEnableVertexAttribArray( 2 );

    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( bspVertex_t ), ( void* ) offsetof( bspVertex_t, position ) );
    glVertexAttribPointer( 1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof( bspVertex_t ), ( void* ) offsetof( bspVertex_t, color ) );
    glVertexAttribPointer( 2, 2, GL_FLOAT, GL_FALSE, sizeof( bspVertex_t ), ( void* ) offsetof( bspVertex_t, texcoord ) );

    glBindVertexArray( 0 );

    glUseProgram( bspProgram );
    glUniformMatrix4fv( glGetUniformLocation( bspProgram, "cameraToClip" ), 1, GL_FALSE, glm::value_ptr( camera.ViewData().projection ) );
    glUseProgram( 0 );
}

/*
=====================================================

BSPRenderer::DrawWorld

=====================================================
*/

void BSPRenderer::DrawWorld( void )
{
    glUseProgram( bspProgram );
    glBindVertexArray( vao );
    glBindBuffer( GL_ARRAY_BUFFER, vbo );

    //glUniform1f( glGetUniformLocation( bspProgram, "deltaTime" ), deltaTime );
    glUniformMatrix4fv( glGetUniformLocation( bspProgram, "modelToCamera" ), 1, GL_FALSE, glm::value_ptr( camera.ViewData().transform ) );

    RenderPass pass( &map, camera.ViewData() );

    DrawNode( 0, pass, true );
    //DrawNode( 0, pass, false );

    glUseProgram( 0 );
    glBindVertexArray( 0 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
}

/*
=====================================================

BSPRenderer::Update

Determine all of the faces which are visible, based on
our current location in the map.

=====================================================
*/

void BSPRenderer::Update( float dt )
{
    deltaTime = dt;

    alreadyVisible.clear();
    visibleFaces.assign( visibleFaces.size(), 0 );

    bspLeaf_t* leaf = map.FindClosestLeaf( camera.ViewData().origin );

    for ( int i = 0; i < map.numLeaves; ++i )
    {
        bspLeaf_t* testLeaf = map.leaves + i;

        if ( map.IsClusterVisible( leaf->clusterIndex, testLeaf->clusterIndex ) )
        {
            for ( int j = testLeaf->leafFaceOffset; j < testLeaf->leafFaceOffset + testLeaf->numLeafFaces; ++j )
            {
                if ( visibleFaces[ map.leafFaces[ j ].index ] == 0 )
                {
                    visibleFaces[ map.leafFaces[ j ].index ] = 1;
                    alreadyVisible.push_back( map.leafFaces[ j ].index );
                }
            }
        }
    }

    currClusterIndex = leaf->clusterIndex;
    camera.Update();
}

/*
=====================================================

BSPRenderer::DrawNodes

=====================================================
*/

#define DRAW_MODE GL_TRIANGLES

void BSPRenderer::DrawNode( int nodeIndex, RenderPass& pass, bool isSolid )
{
    if ( nodeIndex < 0 )
    {
        bspLeaf_t* drawLeaf = &map.leaves[ -( nodeIndex + 1 ) ];

        if ( map.IsClusterVisible( currClusterIndex, drawLeaf->clusterIndex ) )
        {
            for ( int i = drawLeaf->leafFaceOffset; i < drawLeaf->leafFaceOffset + drawLeaf->numLeafFaces; ++i )
            {
                if ( pass.facesRendered[ map.leafFaces[ i ].index ] )
                    continue;

                bspFace_t* face = map.faces + map.leafFaces[ i ].index;

                if ( face->type == FACE_TYPE_MESH || face->type == FACE_TYPE_POLYGON )
                {
                    glDrawElements( GL_TRIANGLES, face->numMeshVertexes, GL_UNSIGNED_INT, ( void* ) &map.meshVertexes[ face->meshVertexOffset ].offset );
                }

                pass.facesRendered[ map.leafFaces[ i ].index ] = true;
            }
        }
    }
    else
    {
        const bspNode_t* const node = &map.nodes[ nodeIndex ];
        const bspPlane_t* const plane = &map.planes[ node->plane ];

        float d = glm::dot( pass.view.origin, glm::vec3( plane->normal.x, plane->normal.y, plane->normal.z ) ) - plane->distance;

        if ( ( d >= 0 ) )
        {
            DrawNode( node->children[ 0 ], pass, isSolid );
            DrawNode( node->children[ 1 ], pass, isSolid );
        }
        else
        {
            DrawNode( node->children[ 1 ], pass, isSolid );
            DrawNode( node->children[ 0 ], pass, isSolid );
        }
    }

}

