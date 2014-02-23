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
      deltaTime( 0 ),
      currLeaf( NULL )
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

    map->Read( filepath, 1 );
    map->GenTextures( filepath );

    //visibleFaces.reserve( map->numFaces );
    //alreadyVisible.reserve( map->numFaces );

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
    pass.cluster = map->FindClosestLeaf( pass.view.origin )->clusterIndex;

    DrawNode( 0, pass, true );
    DrawNode( 0, pass, false );

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
/*
    alreadyVisible.clear();
    //alreadyVisible.assign( map->numFaces, 0 );

    currLeaf = map->FindClosestLeaf( camera->ViewData().origin );

    for ( int i = 0; i < map->numLeaves; ++i )
    {
        bspLeaf_t* test = map->leaves + i;

        if ( map->IsClusterVisible( currLeaf->clusterIndex, test->clusterIndex ) )
        {
            for ( int i = 0; i < test->numLeafFaces; ++i )
            {
                alreadyVisible.push_back( map->leafFaces[ currLeaf->leafFaceOffset + i ].index );
            }
        }
    }
*/

}

void BSPRenderer::DrawIterative( RenderPass& pass )
{
    glm::vec3 max( currLeaf->boxMax.x, currLeaf->boxMax.y, currLeaf->boxMax.z );
    glm::vec3 min( currLeaf->boxMin.x, currLeaf->boxMin.y, currLeaf->boxMin.z );

    AABB bounds( max, min );

    const int alreadyVisLen = ( int ) alreadyVisible.size();

    for ( int i = 0; i < alreadyVisLen; ++i )
    {
        int faceIndex = alreadyVisible[ i ];

        if ( pass.facesRendered[ faceIndex ] )
            continue;

        bspFace_t* face = map->faces + faceIndex;

        if ( ( face->type == FACE_TYPE_MESH || face->type == FACE_TYPE_POLYGON ) && frustum->IntersectsBox( bounds ) )
        {
            glActiveTexture( GL_TEXTURE0 + i );
            glBindTexture( GL_TEXTURE_2D, map->GetApiTexture( face->texture ) );
            glUniform1i( glGetUniformLocation( bspProgram, "texSampler" ), i );

            glDrawElements( GL_TRIANGLES, face->numMeshVertexes, GL_UNSIGNED_INT, ( void* ) &map->meshVertexes[ face->meshVertexOffset ] );

            pass.facesRendered[ faceIndex ] = true;
        }
    }
}

/*
=====================================================

BSPRenderer::DrawNodes

=====================================================
*/

void BSPRenderer::DrawNode( int nodeIndex, RenderPass& pass, bool isSolid )
{
    if ( nodeIndex < 0 )
    {
        const bspLeaf_t* const viewLeaf = &map->leaves[ -( nodeIndex + 1 ) ];

        if ( !map->IsClusterVisible( pass.cluster, viewLeaf->clusterIndex ) ) return;

        /*
        {
            glm::vec3 max( viewLeaf->boxMax.x, viewLeaf->boxMax.y, viewLeaf->boxMin.z );
            glm::vec3 min( viewLeaf->boxMin.x, viewLeaf->boxMin.y, viewLeaf->boxMax.z );

            AABB bounds( max, min );

            if ( !frustum->IntersectsBox( bounds ) ) return;
        }
        */

        for ( int i = 0; i < viewLeaf->numLeafFaces; ++i )
        {
            int index = map->leafFaces[ viewLeaf->leafFaceOffset + i ].index;

            if ( pass.facesRendered[ index ] ) continue;

            DrawFace( index, i, pass, isSolid );
        }

        glBindTexture( GL_TEXTURE_2D, 0 );
    }
    else
    {
        const bspNode_t* const node = &map->nodes[ nodeIndex ];
        const bspPlane_t* const plane = &map->planes[ node->plane ];

        float d = glm::dot( pass.view.origin, glm::vec3( plane->normal.x, plane->normal.y, plane->normal.z ) ) - plane->distance;

        if ( ( d >= 0 ) == isSolid )
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

void BSPRenderer::DrawFace( int faceIndex, int texUnit, RenderPass& pass, bool isSolid )
{
    bspFace_t* face = map->faces + faceIndex;

    glActiveTexture( GL_TEXTURE0 + texUnit );
    glBindTexture( GL_TEXTURE_2D, map->GetApiTexture( face->texture ) );
    glUniform1i( glGetUniformLocation( bspProgram, "texSampler" ), texUnit );

    glDrawElements( GL_TRIANGLES, face->numMeshVertexes, GL_UNSIGNED_INT, ( void* ) &map->meshVertexes[ face->meshVertexOffset ] );

    pass.facesRendered[ faceIndex ] = true;
}

