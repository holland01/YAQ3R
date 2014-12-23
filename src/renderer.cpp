#include "renderer.h"
#include "shader.h"
#include "log.h"
#include "math_util.h"
#include "aabb.h"
#include "glutil.h"

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
    facesRendered.assign( map->numFaces, 0 );
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
      currLeaf( NULL ),
	  mapDimsLength( 0 )
{
	viewParams_t view;
	view.inverseOrient = glm::mat4( 
		glm::vec4( 1.00000000f, -0.000000000f, -1.06630473e-008f, 0.0f ),
		glm::vec4( -7.25249971e-010f, 0.997684300f, -0.0680152625f, 0.0f ),
		glm::vec4( 1.06383551e-008f, 0.0680152625f, 0.997684300f, 0.0f ),
		glm::vec4( -0.000000000f, 0.000000000f, -0.000000000f, 1.0f )
	);

	view.origin = glm::vec3( 1281.95422f, 2441.09473f, -3001.82715f );

	EuAng rot;
	rot.pitch = -3001.82715f;
	rot.yaw = -6.10947609e-007f;
	rot.roll = 0.0f;

    camera = new InputCamera( view, rot );
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

    GL_CHECK( glBindVertexArray( vao ) );

    GL_CHECK( glBindBuffer( GL_ARRAY_BUFFER, vbo ) );
    GL_CHECK( glBufferData( GL_ARRAY_BUFFER, sizeof( bspVertex_t ) * map->numVertexes, map->vertexes, GL_STATIC_DRAW ) );

    GL_CHECK( glBindAttribLocation( bspProgram, 0, "position" ) );
    GL_CHECK( glBindAttribLocation( bspProgram, 1, "color" ) );
    GL_CHECK( glBindAttribLocation( bspProgram, 2, "tex0" ) );

	LoadVertexLayout();

    GL_CHECK( glBindVertexArray( 0 ) );

    GL_CHECK( glUseProgram( bspProgram ) );
    GL_CHECK( glUniformMatrix4fv( glGetUniformLocation( bspProgram, "cameraToClip" ), 1, GL_FALSE, glm::value_ptr( camera->ViewData().clipTransform ) ) );

	mapDimsLength = ( int ) glm::length( glm::vec3( map->nodes[ 0 ].boxMax.x, map->nodes[ 0 ].boxMax.y, map->nodes[ 0 ].boxMax.z ) );
	lodThreshold = mapDimsLength / 3;
}

/*
=====================================================

BSPRenderer::DrawWorld

=====================================================
*/

void BSPRenderer::DrawWorld( void )
{
    GL_CHECK( glBindVertexArray( vao ) ); 
    GL_CHECK( glBindBuffer( GL_ARRAY_BUFFER, vbo ) );

    GL_CHECK( glUniformMatrix4fv( glGetUniformLocation( bspProgram, "modelToCamera" ), 1, GL_FALSE, glm::value_ptr( camera->ViewData().transform ) ) );

    RenderPass pass( map, camera->ViewData() );
    pass.leaf = map->FindClosestLeaf( pass.view.origin );

    DrawNode( 0, pass, true );

	//frustum->PrintMetrics();
	//frustum->ResetMetrics();

    GL_CHECK( glBindVertexArray( 0 ) );
    GL_CHECK( glBindBuffer( GL_ARRAY_BUFFER, 0 ) );
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

void BSPRenderer::DrawNode( int nodeIndex, RenderPass& pass, bool isSolid )
{
    if ( nodeIndex < 0 )
    {
        const bspLeaf_t* const viewLeaf = &map->leaves[ -( nodeIndex + 1 ) ];

        if ( !map->IsClusterVisible( pass.leaf->clusterIndex, viewLeaf->clusterIndex ) )
            return;

        AABB bounds( 
			glm::vec3( viewLeaf->boxMax.x, viewLeaf->boxMax.y, viewLeaf->boxMax.z ), 
			glm::vec3( viewLeaf->boxMin.x, viewLeaf->boxMin.y, viewLeaf->boxMin.z ) 
		);

        if ( !frustum->IntersectsBox( bounds ) )
            return;

        for ( int i = 0; i < viewLeaf->numLeafFaces; ++i )
        {
            int index = map->leafFaces[ viewLeaf->leafFaceOffset + i ].index;

            if ( pass.facesRendered[ index ] )
                continue;

            DrawFace( index, pass, bounds, isSolid );
        }

		GL_CHECK( glBindTexture( GL_TEXTURE_2D, 0 ) );
    }
    else
    {
        const bspNode_t* const node = &map->nodes[ nodeIndex ];
        const bspPlane_t* const plane = &map->planes[ node->plane ];

        float d = glm::dot( pass.view.origin, glm::vec3( plane->normal.x, plane->normal.y, plane->normal.z ) );

        if ( ( d > plane->distance ) == isSolid )
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

void BSPRenderer::DrawFace( int faceIndex, RenderPass& pass, const AABB& bounds, bool isSolid )
{
    bspFace_t* face = map->faces + faceIndex;

	std::vector< GLuint > indices;

	if ( face->texture >= 0 )
	{
		GL_CHECK( glActiveTexture( GL_TEXTURE0 ) );
		GL_CHECK( glBindTexture( GL_TEXTURE_2D, map->glTextures[ face->texture ] ) );
		GL_CHECK( glUniform1i( glGetUniformLocation( bspProgram, "texSampler" ), 0 ) );
	}
	else
	{
		GL_CHECK( glBindTexture( GL_TEXTURE_2D, 0 ) );
	}

	if ( face->type == FACE_TYPE_POLYGON || face->type == FACE_TYPE_MESH )
	{
		indices.reserve( face->numMeshVertexes );
		for ( int i = 0; i < face->numMeshVertexes; ++i )
		{
			indices.push_back( face->vertexOffset + map->meshVertexes[ face->meshVertexOffset + i ].offset );
		}

		GL_CHECK( glDrawElements( GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, &indices[ 0 ] ) );
	}
	else if ( face->type == FACE_TYPE_PATCH )
	{
		// The amount of increments we need to make for each dimension, so we have the (potentially) shared points between patches
		int stepWidth = ( face->size[ 0 ] - 1 ) / 2;
		int stepHeight = ( face->size[ 1 ] - 1 ) / 2;

		int c = 0;
		for ( int i = 0; i < face->size[ 0 ]; i += stepWidth )
			for ( int j = 0; j < face->size[ 1 ]; j += stepHeight )
				patchRenderer.controlPoints[ c++ ] = &map->vertexes[ face->vertexOffset + j * face->size[ 0 ] + i ];	
				
		patchRenderer.Tesselate( CalcSubdivision( pass, bounds ) );
		patchRenderer.Render();

		// Rebind after render since patchRenderer overrides with its own vao/vbo combo
		GL_CHECK( glBindVertexArray( vao ) );
		GL_CHECK( glBindBuffer( GL_ARRAY_BUFFER, vbo ) );
	}

    pass.facesRendered[ faceIndex ] = 1;
}


int BSPRenderer::CalcSubdivision( const RenderPass& pass, const AABB& bounds )
{
	int min = INT_MAX;

	// Find the closest point to the camera
	for ( int i = 0; i < 8; ++i )
	{
		int d = ( int ) glm::distance( pass.view.origin, bounds.Corner( i ) );
		if ( min > d )
			min = d;
	}

	// Compute our subdivision level based on the length of the map's size vector
	// and its ratio in relation with the closest distance
	int subdiv = 0;
	if ( min > lodThreshold )
		subdiv = 1;
	else
		subdiv = mapDimsLength / min;

	return subdiv;
}

