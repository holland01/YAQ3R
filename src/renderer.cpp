#include "renderer.h"
#include "shader.h"
#include "log.h"
#include "math_util.h"
#include "aabb.h"
#include "glutil.h"

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
	  drawDebugInfo( false ),
	  gammaCorrectVertexColors( true ),
      map( NULL ),
      bspProgram( 0 ),
      vao( 0 ),
      vbo( 0 ),
      deltaTime( 0 ),
      currLeaf( NULL ),
	  mapDimsLength( 0 )
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
		
		"doGammaCorrect",
		"modelToCamera",
		"cameraToClip"
	};

	for ( size_t i = 0; i < uniforms.size(); ++i )
		bspProgramUniforms[ uniforms[ i ] ] = GL_CHECK( glGetUniformLocation( bspProgram, uniforms[ i ].c_str() ) );
	
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
        map->DestroyMap();

    map->Read( filepath, 1 );

	// Allocate vertex data from map and store it all in a single vbo
	GL_CHECK( glBindVertexArray( vao ) );

	GL_CHECK( glBindBuffer( GL_ARRAY_BUFFER, vbo ) );
    GL_CHECK( glBufferData( GL_ARRAY_BUFFER, sizeof( bspVertex_t ) * map->numVertexes, map->vertexes, GL_STATIC_DRAW ) );

    GL_CHECK( glBindAttribLocation( bspProgram, 0, "position" ) );
    GL_CHECK( glBindAttribLocation( bspProgram, 1, "color" ) );
    GL_CHECK( glBindAttribLocation( bspProgram, 2, "tex0" ) );
	GL_CHECK( glBindAttribLocation( bspProgram, 3, "lightmap" ) );

	LoadVertexLayout();

	// Load projection transform
    GL_CHECK( glUseProgram( bspProgram ) );
    GL_CHECK( glUniformMatrix4fv( bspProgramUniforms[ "cameraToClip" ], 1, GL_FALSE, glm::value_ptr( camera->ViewData().clipTransform ) ) );

	// Base texture setup
	
	//GL_CHECK( glUniform1i( bspProgramUniforms[ "fragTexSampler" ], 0 ) );

	mapDimsLength = ( int ) glm::length( glm::vec3( map->nodes[ 0 ].boxMax.x, map->nodes[ 0 ].boxMax.y, map->nodes[ 0 ].boxMax.z ) );
	lodThreshold = mapDimsLength / 2;
}

/*
=====================================================

BSPRenderer::DrawWorld

=====================================================
*/

void BSPRenderer::DrawWorld( void )
{ 
	GL_CHECK( glUniform1i( bspProgramUniforms[ "doGammaCorrect" ], gammaCorrectVertexColors ? 1 : 0 ) );

	double startTime = glfwGetTime();
	GL_CHECK( glUniformMatrix4fv( bspProgramUniforms[ "modelToCamera" ], 1, GL_FALSE, glm::value_ptr( camera->ViewData().transform ) ) );

    RenderPass pass( map, camera->ViewData() );
    pass.leaf = map->FindClosestLeaf( pass.view.origin );

    DrawNode( 0, pass, true );
	frameTime = glfwGetTime() - startTime;
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
    }
    else
    {
        const bspNode_t* const node = &map->nodes[ nodeIndex ];
        const bspPlane_t* const plane = &map->planes[ node->plane ];

        float d = glm::dot( pass.view.origin, glm::vec3( plane->normal.x, plane->normal.y, plane->normal.z ) );

		// We're in front of the plane if d > plane->distance
        if ( isSolid == ( d > plane->distance ) )
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

static const float ONE_THIRD = 1.0f / 3.0f;

void BSPRenderer::DrawFace( int faceIndex, RenderPass& pass, const AABB& bounds, bool isSolid )
{
    bspFace_t* face = map->faces + faceIndex;

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
				patchRenderer.controlPoints[ c++ ] = &map->vertexes[ face->vertexOffset + j * face->size[ 0 ] + i ];	
				
		patchRenderer.Tesselate( CalcSubdivision( pass, bounds ) );
		patchRenderer.Render();

		// Rebind after render since patchRenderer overrides with its own vao/vbo combo
		LoadBuffer( vbo );
	}

	if ( drawDebugInfo )
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


		//GL_CHECK( glLoadMatrixf( glm::value_ptr( camera->ViewData().transform ) ) );
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

