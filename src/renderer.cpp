#include "renderer.h"
#include "shader.h"
#include "log.h"
#include "math_util.h"
#include "aabb.h"

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

RenderPass::RenderPass( const Q3BspMap* const& map, const viewParams_t& viewData )
    : view( viewData )
{
    facesRendered.reserve( map->numFaces );
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
    : camera( NULL ),
      frustum( NULL ),
      map( NULL ),
      bspProgram( 0 ),
      vao( 0 ),
      vbo( 0 ),
      deltaTime( 0 )
{
    camera = new InputCamera();
    frustum = new Frustum();
    map = new Q3BspMap();
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

    delete map;
    delete frustum;
    delete camera;
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
    if ( map->IsAllocated() )
    {
        map->DestroyMap();
    }

    map->Read( filepath );
    map->GenTextures( filepath );
    visibleFaces.reserve( map->numFaces );

    glBindVertexArray( vao );

    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    glBufferData( GL_ARRAY_BUFFER, sizeof( bspVertex_t ) * map->numVertexes, map->vertexes, GL_STATIC_DRAW );

    glBindAttribLocation( bspProgram, 0, "position" );
    glBindAttribLocation( bspProgram, 1, "color" );
    glBindAttribLocation( bspProgram, 2, "tex0" );

    glEnableVertexAttribArray( 0 );
    glEnableVertexAttribArray( 1 );
    glEnableVertexAttribArray( 2 );

    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( bspVertex_t ), ( void* ) offsetof( bspVertex_t, position ) );
    glVertexAttribPointer( 1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof( bspVertex_t ), ( void* ) offsetof( bspVertex_t, color ) );
    glVertexAttribPointer( 2, 2, GL_FLOAT, GL_FALSE, sizeof( bspVertex_t ), ( void* ) offsetof( bspVertex_t, texCoord ) );

    glBindVertexArray( 0 );

    glUseProgram( bspProgram );
    glUniformMatrix4fv( glGetUniformLocation( bspProgram, "cameraToClip" ), 1, GL_FALSE, glm::value_ptr( camera->ViewData().projection ) );
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
    glUniformMatrix4fv( glGetUniformLocation( bspProgram, "modelToCamera" ), 1, GL_FALSE, glm::value_ptr( camera->ViewData().transform ) );

    RenderPass pass( map, camera->ViewData() );

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
our current location in the map

=====================================================
*/

void BSPRenderer::Update( float dt )
{
    deltaTime = dt;
    camera->Update();
    frustum->Update( camera->ViewData() );
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
        bspLeaf_t* drawLeaf = &map->leaves[ -( nodeIndex + 1 ) ];

        for ( int i = 0; i < map->numLeaves; ++i )
        {
            bspLeaf_t* testLeaf = &map->leaves[ i ];

            if ( map->IsClusterVisible( drawLeaf->clusterIndex, testLeaf->clusterIndex ) )
            {
                for ( int k = 0; k < testLeaf->numLeafFaces; ++k )
                {
                    int faceIndex = map->leafFaces[ testLeaf->leafFaceOffset + k ].index;

                    if ( pass.facesRendered[ faceIndex ] )
                        continue;

                    glm::vec3 max( testLeaf->boxMax.x, testLeaf->boxMax.y, testLeaf->boxMax.z );
                    glm::vec3 min( testLeaf->boxMin.x, testLeaf->boxMin.y, testLeaf->boxMin.z );

                    AABB bounds( max, min );

                    DrawFace( map->leafFaces[ faceIndex ].index, k, pass, bounds, isSolid );
                }

            }
        }

        glBindTexture( GL_TEXTURE_2D, 0 );
    }
    else
    {
        const bspNode_t* const node = &map->nodes[ nodeIndex ];
        const bspPlane_t* const plane = &map->planes[ node->plane ];

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

void BSPRenderer::DrawFace( int faceIndex, int texUnit, RenderPass& pass, const AABB& bounds, bool isSolid )
{
    if ( frustum->IntersectsBox( bounds ) )
    {
        bspFace_t* face = map->faces + faceIndex;

        glActiveTexture( GL_TEXTURE0 + texUnit );
        glBindTexture( GL_TEXTURE_2D, map->GetApiTexture( face->texture ) );
        glUniform1i( glGetUniformLocation( bspProgram, "texSampler" ), texUnit );

        glDrawElements( GL_TRIANGLES, face->numMeshVertexes, GL_UNSIGNED_INT, ( void* ) &map->meshVertexes[ face->meshVertexOffset ] );

        pass.facesRendered[ faceIndex ] = true;
    }
}

