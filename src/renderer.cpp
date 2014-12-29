#include "renderer.h"
#include "shader.h"
#include "log.h"
#include "math_util.h"
#include "aabb.h"
#include "glutil.h"
#include <glm/gtx/string_cast.hpp>

using namespace std;

enum 
{
	FRAGWRITE_TEX = 0,
	FRAGWRITE_TEX_COLOR = 1,
	FRAGWRITE_COLOR = 2
};

/*
=====================================================

RenderPass::RenderPass

=====================================================
*/

RenderPass::RenderPass( const Q3BspMap* const& map, const viewParams_t& viewData )
    : view( viewData )
{
    facesRendered.resize( map->data.numFaces, 0 );
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
	  map ( NULL ),
      bspProgram( 0 ),
      vao( 0 ),
      vbo( 0 ),
      deltaTime( 0 ),
      currLeaf( NULL ),
	  mapDimsLength( 0 )
{
	viewParams_t view;
	view.origin = glm::vec3( -131.291901f, -61.794476f, -163.203659f ); /// debug position which doesn't kill framerate

	camera = new InputCamera( view, EuAng() );

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
	glDeleteBuffers( 1, &vbo );
    glDeleteProgram( bspProgram );
    
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
	GL_CHECK( glPointSize( 10.0f ) );
	// GL_CHECK( glLineWidth( 10.0f ) );

    glGenVertexArrays( 1, &vao );
    glGenBuffers( 1, &vbo );

    GLuint shaders[] =
    {
        CompileShader( "src/main.vert", GL_VERTEX_SHADER ),
        CompileShader( "src/main.frag", GL_FRAGMENT_SHADER )
    };

    bspProgram = LinkProgram( shaders, 2 );

	const std::vector< std::string > uniforms = 
	{
		"fragWriteMode",
		"fragTexSampler",
		"fragLightmapSampler",
		"fragAmbient",
		
		"modelToCamera",
		"cameraToClip"
	};

	for ( size_t i = 0; i < uniforms.size(); ++i )
		bspProgramUniforms[ uniforms[ i ] ] = GL_CHECK( glGetUniformLocation( bspProgram, uniforms[ i ].c_str() ) );
	
	GL_CHECK( glBindAttribLocation( bspProgram, 0, "position" ) );
    GL_CHECK( glBindAttribLocation( bspProgram, 1, "color" ) );
    GL_CHECK( glBindAttribLocation( bspProgram, 2, "tex0" ) );
	GL_CHECK( glBindAttribLocation( bspProgram, 3, "lightmap" ) );

	// Load projection transform
    GL_CHECK( glUseProgram( bspProgram ) );
    GL_CHECK( glUniformMatrix4fv( bspProgramUniforms[ "cameraToClip" ], 1, GL_FALSE, glm::value_ptr( camera->ViewData().clipTransform ) ) );

    glUseProgram( bspProgram );
}

/*
=====================================================

BSPRenderer::Load

    Load map file specified by param filepath.

=====================================================
*/

void BSPRenderer::Load( const string& filepath, uint32_t mapLoadFlags )
{
    map->Read( filepath, 1, mapLoadFlags );

	// Allocate vertex data from map and store it all in a single vbo
	GL_CHECK( glBindVertexArray( vao ) );

	GL_CHECK( glBindBuffer( GL_ARRAY_BUFFER, vbo ) );
    GL_CHECK( glBufferData( GL_ARRAY_BUFFER, sizeof( bspVertex_t ) * map->data.numVertexes, map->data.vertexes, GL_STATIC_DRAW ) );

	LoadVertexLayout();

	const bspNode_t* root = &map->data.nodes[ 0 ];

	// Base texture setup
	mapDimsLength = ( int ) glm::length( glm::vec3( root->boxMax.x, root->boxMax.y, root->boxMax.z ) );
	lodThreshold = mapDimsLength / 2;
}

/*
=====================================================

BSPRenderer::DrawWorld

=====================================================
*/

void BSPRenderer::Render( uint32_t renderFlags )
{ 
	double startTime = glfwGetTime();
	GL_CHECK( glUniformMatrix4fv( bspProgramUniforms[ "modelToCamera" ], 1, GL_FALSE, glm::value_ptr( camera->ViewData().transform ) ) );

    RenderPass pass( map, camera->ViewData() );
    pass.leaf = map->FindClosestLeaf( pass.view.origin );

    DrawNode( 0, pass, true, renderFlags );
	frameTime = glfwGetTime() - startTime;

	MyPrintf( "Cam Pos", "%s", glm::to_string( pass.view.origin ) );
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

void BSPRenderer::DrawNode( int nodeIndex, RenderPass& pass, bool isSolid, uint32_t renderFlags )
{
    if ( nodeIndex < 0 )
    {
        const bspLeaf_t* const viewLeaf = &map->data.leaves[ -( nodeIndex + 1 ) ];

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
            int index = map->data.leafFaces[ viewLeaf->leafFaceOffset + i ].index;

            if ( pass.facesRendered[ index ] )
                continue;

            DrawFace( index, pass, bounds, isSolid, renderFlags );
        }
    }
    else
    {
        const bspNode_t* const node = &map->data.nodes[ nodeIndex ];
        const bspPlane_t* const plane = &map->data.planes[ node->plane ];

        float d = glm::dot( pass.view.origin, glm::vec3( plane->normal.x, plane->normal.y, plane->normal.z ) );

		// We're in front of the plane if d > plane->distance.
		// If both of these are true, it makes sense to draw what is in front of us, as any 
		// non-solid object can be handled properly by depth if it's infront of the partition plane
		// and we're behind it
        if ( isSolid == ( d > plane->distance ) )
        {
            DrawNode( node->children[ 0 ], pass, isSolid, renderFlags );
            DrawNode( node->children[ 1 ], pass, isSolid, renderFlags );
        }
        else
        {
            DrawNode( node->children[ 1 ], pass, isSolid, renderFlags );
            DrawNode( node->children[ 0 ], pass, isSolid, renderFlags );
        }
    }
}

static const float ONE_THIRD = 1.0f / 3.0f;

void BSPRenderer::DrawFace( int faceIndex, RenderPass& pass, const AABB& bounds, bool isSolid, uint32_t renderFlags )
{
    bspFace_t* face = map->data.faces + faceIndex;

    GL_CHECK( glActiveTexture( GL_TEXTURE0 ) );
    GL_CHECK( glUniform1i( bspProgramUniforms[ "fragTexSampler" ], 0 ) );

	if ( map->glTextures[ face->texture ] != 0 )
	{
		GL_CHECK( glBindTexture( GL_TEXTURE_2D, map->glTextures[ face->texture ] ) );
		GL_CHECK( glUniform1i( bspProgramUniforms[ "fragWriteMode" ], FRAGWRITE_TEX_COLOR ) );
	}
	else
	{
		GL_CHECK( glUniform1i( bspProgramUniforms[ "fragWriteMode" ], FRAGWRITE_COLOR ) );
		GL_CHECK( glBindTexture( GL_TEXTURE_2D, 0 ) );
	}

	if ( face->lightmapIndex >= 0 )
	{
		GL_CHECK( glActiveTexture( GL_TEXTURE0 + 1 ) );
		GL_CHECK( glUniform1i( bspProgramUniforms[ "fragLightmapSampler" ], 1 ) );

		GL_CHECK( glBindTexture( GL_TEXTURE_2D, map->glLightmaps[ face->lightmapIndex ] ) );
		GL_CHECK( glUniform1i( bspProgramUniforms[ "fragWriteMode" ], FRAGWRITE_TEX_COLOR ) );
	}
	
	if ( face->type == BSP_FACE_TYPE_POLYGON || face->type == BSP_FACE_TYPE_MESH )
	{
		GL_CHECK( glDrawElements( GL_TRIANGLES, map->glFaces[ faceIndex ].indices.size(), GL_UNSIGNED_INT, &map->glFaces[ faceIndex ].indices[ 0 ] ) );
	}
	else if ( face->type == BSP_FACE_TYPE_PATCH )
	{
		// The amount of increments we need to make for each dimension, so we have the (potentially) shared points between patches
		int stepWidth = ( face->size[ 0 ] - 1 ) / 2;
		int stepHeight = ( face->size[ 1 ] - 1 ) / 2;

		int c = 0;
		for ( int i = 0; i < face->size[ 0 ]; i += stepWidth )
			for ( int j = 0; j < face->size[ 1 ]; j += stepHeight )
				patchRenderer.controlPoints[ c++ ] = &map->data.vertexes[ face->vertexOffset + j * face->size[ 0 ] + i ];	
				
		patchRenderer.Tesselate( CalcSubdivision( pass, bounds ) );
		patchRenderer.Render();

		// Rebind after render since patchRenderer overrides with its own vao/vbo combo
		LoadBuffer( vbo );
	}
	else // Billboards
	{
		// TODO
		__nop();
	}

	// Evaluate optional settings
	if ( renderFlags & RENDER_BSP_LIGHTMAP_INFO )
	{
		GL_CHECK( glUseProgram( 0 ) );

		GL_CHECK( glMatrixMode( GL_PROJECTION ) );
		GL_CHECK( glLoadMatrixf( glm::value_ptr( camera->ViewData().clipTransform ) ) );

		GL_CHECK( glMatrixMode( GL_MODELVIEW ) );

		glm::mat4 viewTriOrigin( camera->ViewData().transform );

		GL_CHECK( glLoadMatrixf( glm::value_ptr( viewTriOrigin ) ) );
		GL_CHECK( glBegin( GL_LINES ) );

		glm::vec3 s( face->lightmapStVecs[ 0 ].x, face->lightmapStVecs[ 0 ].y, face->lightmapStVecs[ 0 ].z );
		glm::vec3 t( face->lightmapStVecs[ 1 ].x, face->lightmapStVecs[ 1 ].y, face->lightmapStVecs[ 1 ].z );

		// Draw lightmap s vector
		GL_CHECK( glColor3f( 1.0f, 0.0f, 0.0f ) );
		GL_CHECK( glVertex3f( 0.0f, 0.0f, 0.0f ) );
		GL_CHECK( glVertex3f( s.x, s.y, s.z ) ); 

		// Draw lightmap t vector
		GL_CHECK( glColor3f( 0.0f, 1.0f, 0.0f ) );
		GL_CHECK( glVertex3f( 0.0f, 0.0f, 0.0f ) );
		GL_CHECK( glVertex3f( t.x, t.y, t.z ) ); 
		
		GL_CHECK( glEnd() );

		// Draw lightmap origin
		GL_CHECK( glBegin( GL_POINTS ) );
		GL_CHECK( glColor3f( 0.0f, 0.0f, 1.0f ) );
		GL_CHECK( glVertex3f( face->lightmapOrigin.x, face->lightmapOrigin.y, face->lightmapOrigin.z ) );
		GL_CHECK( glEnd() );
		
		GL_CHECK( glLoadIdentity() );

		GL_CHECK( glUseProgram( bspProgram ) );
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