#include "renderer.h"
#include "shader.h"
#include "log.h"
#include "math_util.h"

enum
{
    FACE_TYPE_BILLBOAD = 4,
    FACE_TYPE_MESH = 3,
    FACE_TYPE_PATCH = 2,
    FACE_TYPE_POLYGON = 1
};

namespace
{
    bool debug_drawTarget = true;

    GLuint targetVbo, targetVao;
    GLuint targetProgram;

    const float quadVertices[] =
    {
        // positions

        0.5f,  0.5f,  1.0f,
        0.5f,  0.5f,  1.0f,
        0.5f, -0.5f,  1.0f,
        0.5f, -0.5f,  1.0f,
        0.5f, -0.5f,  1.0f,
        0.5f,  0.5f,  1.0f,

        // colors
        1.0f, 1.0f, 1.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 1.0f,
    };

    void InitDrawTargetData( void )
    {
        GLuint shaders[] =
        {
            CompileShader( "src/targetDraw.vert", GL_VERTEX_SHADER ),
            CompileShader( "src/vertexColor.frag", GL_FRAGMENT_SHADER )
        };

        targetProgram = LinkProgram( shaders, 2 );

        glGenVertexArrays( 1, &targetVao );
        glBindVertexArray( targetVao );

        glGenBuffers( 1, &targetVbo );
        glBindBuffer( GL_ARRAY_BUFFER, targetVbo );
        glBufferData( GL_ARRAY_BUFFER, sizeof( quadVertices ), quadVertices, GL_STATIC_DRAW );

        GLuint posAttrib = glGetAttribLocation( targetProgram, "position" );
        GLuint colAttrib = glGetAttribLocation( targetProgram, "color" );

        const size_t colorOffset = sizeof( quadVertices ) / 2;

        glEnableVertexAttribArray( posAttrib );
        glEnableVertexAttribArray( colAttrib );
        glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 0, ( void* ) 0 );
        glVertexAttribPointer( colAttrib, 3, GL_FLOAT, GL_FALSE, 0, ( void* ) colorOffset );

        glBindVertexArray( 0 );

        glBindBuffer( GL_ARRAY_BUFFER, 0 );
    }

    void DrawTarget( const glm::vec3& targetPos, const glm::vec3& camPos, const glm::mat4& projection )
    {
        glm::mat4 view( 1.0f );

        glUseProgram( targetProgram );
        glBindVertexArray( targetVao );
        glBindBuffer( GL_ARRAY_BUFFER, targetVbo );

        const glm::vec3& axis = targetPos - camPos;

        view = glm::translate( view, glm::vec3( 0.0f, 0.0f, -glm::length( axis ) ) );

        glUniformMatrix4fv( glGetUniformLocation( targetProgram, "model" ), 1, GL_FALSE, glm::value_ptr( view ) );
        glUniformMatrix4fv( glGetUniformLocation( targetProgram, "projection" ), 1, GL_FALSE, glm::value_ptr( projection ) );

        glDrawArrays( GL_TRIANGLES, 0, 6 );

        glUseProgram( 0 );
        glBindVertexArray( 0 );
        glUseProgram( 0 );
    }
}

/*
=====================================================

RenderPass::RenderPass

=====================================================
*/

RenderPass::RenderPass( const Quake3Map* const map, const ViewParams& viewData )
    : view( viewData ),
      facesRendered( map->numFaces )
{
    facesRendered.assign( map->numFaces, false );
}

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
    InitDrawTargetData();

    glGenVertexArrays( 1, &vao );
    glGenBuffers( 1, &vbo );
    glGenTextures( 1, &texObj );

    GLuint shaders[] =
    {
        CompileShader( "src/main.vert", GL_VERTEX_SHADER ),
        CompileShader( "src/vertexColor.frag", GL_FRAGMENT_SHADER )
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
    }

    map.Read( filepath );
    visibleFaces.reserve( map.numFaces );

    glBindVertexArray( vao );

    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, texObj );
    //glTexImage2D(

    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    glBufferData( GL_ARRAY_BUFFER, sizeof( BSPVertex ) * map.numVertexes, map.vertexes, GL_STATIC_DRAW );

    glBindAttribLocation( bspProgram, 0, "position" );
    glBindAttribLocation( bspProgram, 1, "color" );
    glBindAttribLocation( bspProgram, 2, "tex0" );

    glEnableVertexAttribArray( 0 );
    glEnableVertexAttribArray( 1 );
    glEnableVertexAttribArray( 2 );

    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( BSPVertex ), ( void* ) offsetof( BSPVertex, position ) );
    glVertexAttribPointer( 1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof( BSPVertex ), ( void* ) offsetof( BSPVertex, color ) );
    glVertexAttribPointer( 2, 2, GL_FLOAT, GL_FALSE, sizeof( BSPVertex ), ( void* ) offsetof( BSPVertex, texcoord ) );

    glBindVertexArray( 0 );

    glUseProgram( bspProgram );
    glUniformMatrix4fv( glGetUniformLocation( bspProgram, "projection" ), 1, GL_FALSE, glm::value_ptr( camera.ViewData().projection ) );
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

    glUniform1f( glGetUniformLocation( bspProgram, "deltaTime" ), deltaTime );
    glUniformMatrix4fv( glGetUniformLocation( bspProgram, "modelView" ), 1, GL_FALSE, glm::value_ptr( camera.ViewData().transform ) );

    RenderPass pass( &map, camera.ViewData() );

    DrawNode( 0, pass, true );
    DrawNode( 0, pass, false );

    glUseProgram( 0 );
    glBindVertexArray( 0 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
}

void BSPRenderer::Update( float dt )
{
    deltaTime = dt;

    alreadyVisible.clear();
    visibleFaces.assign( visibleFaces.size(), 0 );

    BSPLeaf* leaf = map.FindClosestLeaf( camera.ViewData().origin );

    for ( int i = 0; i < map.numLeaves; ++i )
    {
        BSPLeaf* testLeaf = map.leaves + i;

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
        BSPLeaf* drawLeaf = &map.leaves[ -( nodeIndex + 1 ) ];

        if ( map.IsClusterVisible( currClusterIndex, drawLeaf->clusterIndex ) )
        {
            for ( int i = drawLeaf->leafFaceOffset; i < drawLeaf->leafFaceOffset + drawLeaf->numLeafFaces; ++i )
            {
                if ( pass.facesRendered[ map.leafFaces[ i ].index ] )
                    continue;

                BSPFace* face = map.faces + map.leafFaces[ i ].index;

                if ( face->type == FACE_TYPE_MESH || face->type == FACE_TYPE_POLYGON )
                {
                    glDrawElements( GL_TRIANGLES, face->numMeshVertexes, GL_UNSIGNED_INT, ( void* ) &map.meshVertexes[ face->meshVertexOffset ].offset );
                }

                pass.facesRendered[ map.leafFaces[ i ].index ] = true;
            }
        }

        return;
    }

    const BSPNode* const node = &map.nodes[ nodeIndex ];
    const BSPPlane* const plane = &map.planes[ node->plane ];

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

