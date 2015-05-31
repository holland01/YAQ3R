#include "renderer.h"
#include "shader.h"
#include "log.h"
#include "math_util.h"
#include "effect_shader.h"
#include "deform.h"
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
    : camera( nullptr ),
      frustum( new Frustum() ),
	  mapDimsLength( 0 ),
	  transformBlockIndex( 0 ),
	  transformBlockObj( 0 ),
      transformBlockSize( sizeof( glm::mat4 ) * 2 ),
      map ( new Q3BspMap() ),
	  currLeaf( nullptr ),
      bspProgram( 0 ),
      vao( 0 ),
      vbo( 0 ),
      deltaTime( 0.0 ),
      frameTime( 0.0f )
{
	viewParams_t view;
	view.origin = glm::vec3( -131.291901f, -61.794476f, -163.203659f ); /// debug position which doesn't kill framerate

	camera = new InputCamera( view, EuAng() );
}

/*
=====================================================

BSPRenderer::~BSPRenderer

=====================================================
*/

BSPRenderer::~BSPRenderer( void )
{
    GL_CHECK( glDeleteVertexArrays( 1, &vao ) );
	GL_CHECK( glDeleteBuffers( 1, &vbo ) );
	GL_CHECK( glDeleteProgram( bspProgram ) );

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
	GL_CHECK( glEnable( GL_DEPTH_TEST ) );
	GL_CHECK( glEnable( GL_BLEND ) );
	GL_CHECK( glEnable( GL_FRAMEBUFFER_SRGB ) );

    GL_CHECK( glDepthFunc( GL_LEQUAL ) );
	GL_CHECK( glBlendEquationSeparate( GL_FUNC_ADD, GL_FUNC_ADD ) );
	
	GL_CHECK( glPointSize( 20.0f ) );
	GL_CHECK( glPolygonOffset( 5.0f, 1.0f ) );

	GL_CHECK( glClearColor( 0.0f, 0.0f, 0.0f, 1.0f ) );

    GL_CHECK( glGenVertexArrays( 1, &vao ) );
    GL_CHECK( glGenBuffers( 1, &vbo ) );
	
	GL_CHECK( glDisable( GL_CULL_FACE ) );

    GLuint shaders[] =
    {
        CompileShader( "src/main.vert", GL_VERTEX_SHADER ),
        CompileShader( "src/main.frag", GL_FRAGMENT_SHADER )
    };

    bspProgram = LinkProgram( shaders, 2 );

	const std::vector< std::string > uniforms = 
	{
		"fragTexSampler",
		"fragLightmapSampler",
		"fragAmbient"
	};

	for ( size_t i = 0; i < uniforms.size(); ++i )
		GL_CHECK( bspProgramUniforms[ uniforms[ i ] ] = glGetUniformLocation( bspProgram, uniforms[ i ].c_str() ) );
	
	GL_CHECK( glBindAttribLocation( bspProgram, 0, "position" ) );
    GL_CHECK( glBindAttribLocation( bspProgram, 1, "color" ) );
    GL_CHECK( glBindAttribLocation( bspProgram, 2, "tex0" ) );
	GL_CHECK( glBindAttribLocation( bspProgram, 3, "lightmap" ) );

	GL_CHECK( glGenBuffers( 1, &transformBlockObj ) );
	GL_CHECK( glBindBuffer( GL_UNIFORM_BUFFER, transformBlockObj ) );
	GL_CHECK( glBufferData( GL_UNIFORM_BUFFER, transformBlockSize, NULL, GL_STREAM_DRAW ) );
	GL_CHECK( glBufferSubData( GL_UNIFORM_BUFFER, 0, sizeof( glm::mat4 ), glm::value_ptr( camera->ViewData().clipTransform ) ) );
	GL_CHECK( glBindBuffer( GL_UNIFORM_BUFFER, 0 ) );

	GL_CHECK( glBindBufferRange( GL_UNIFORM_BUFFER, UBO_TRANSFORMS_BLOCK_BINDING, transformBlockObj, 0, transformBlockSize ) );

	MapProgramToUBO( bspProgram, "Transforms" );
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

	LoadVertexLayout( true );

	const bspNode_t* root = &map->data.nodes[ 0 ];

	// Base texture setup
	mapDimsLength = ( int ) glm::length( glm::vec3( root->boxMax.x, root->boxMax.y, root->boxMax.z ) );
	lodThreshold = mapDimsLength / 2;
}

void BSPRenderer::Render( uint32_t renderFlags )
{ 
	double startTime = glfwGetTime();

	GL_CHECK( glBindBuffer( GL_UNIFORM_BUFFER, transformBlockObj ) );
	GL_CHECK( glBufferSubData( GL_UNIFORM_BUFFER, sizeof( glm::mat4 ), sizeof( glm::mat4 ), glm::value_ptr( camera->ViewData().transform ) ) );

    RenderPass pass( map, camera->ViewData() );

    pass.leaf = map->FindClosestLeaf( pass.view.origin );

    DrawNode( 0, pass, true, renderFlags );
	
	drawFace_t parms = { 0 };
	parms.pass = &pass;

	AABB bounds;

	for ( int i = 1; i < map->data.numModels; ++i )
	{
		bspModel_t* model = &map->data.models[ i ];

		bounds.maxPoint = model->boxMax;
		bounds.minPoint = model->boxMin;

		if ( !frustum->IntersectsBox( bounds ) )
			continue;

		for ( int j = 0; j < model->numFaces; ++j )
		{
			if ( pass.facesRendered[ model->faceOffset + j ] )
				continue;

			parms.faceIndex = model->faceOffset + j;
			parms.face = &map->data.faces[ parms.faceIndex ];
			parms.shader = map->GetShaderInfo( parms.faceIndex );

			DrawFace( &parms );
		}
	}
	
	//DrawNode( 0, pass, false , renderFlags );

	frameTime = glfwGetTime() - startTime;

	GL_CHECK( glBindBuffer( GL_UNIFORM_BUFFER, 0 ) );
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

void BSPRenderer::DrawNode( int nodeIndex, RenderPass& pass, bool isSolid, uint32_t renderFlags )
{
    if ( nodeIndex < 0 )
    {
		drawFace_t parms = { 0 };

        const bspLeaf_t* viewLeaf = &map->data.leaves[ -( nodeIndex + 1 ) ];

        if ( !map->IsClusterVisible( pass.leaf->clusterIndex, viewLeaf->clusterIndex ) )
            return;

		AABB leafBounds;
		leafBounds.maxPoint = glm::vec3( viewLeaf->boxMax.x, viewLeaf->boxMax.y, viewLeaf->boxMax.z );
		leafBounds.minPoint = glm::vec3( viewLeaf->boxMin.x, viewLeaf->boxMin.y, viewLeaf->boxMin.z );

        if ( !frustum->IntersectsBox( leafBounds ) )
            return;

		parms.pass = &pass;
		parms.isSolid = isSolid;
		parms.renderFlags = renderFlags;
		parms.bounds = &leafBounds;

        for ( int i = 0; i < viewLeaf->numLeafFaces; ++i )
        {
            parms.faceIndex = map->data.leafFaces[ viewLeaf->leafFaceOffset + i ].index;

            if ( pass.facesRendered[ parms.faceIndex ] )
                continue;

			parms.face = &map->data.faces[ parms.faceIndex ];
			parms.shader = map->GetShaderInfo( parms.faceIndex );

			DrawFace( &parms );
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

void BSPRenderer::DrawFaceNoEffect( drawFace_t* parms )
{
	GL_CHECK( glActiveTexture( GL_TEXTURE0 ) );
	GL_CHECK( glProgramUniform1i( bspProgram, bspProgramUniforms[ "fragTexSampler" ], 0 ) );

	if ( map->glTextures[ parms->face->texture ] != 0 )
	{
		GL_CHECK( glBindSampler( 0, map->glSamplers[ parms->face->texture ] ) );
		GL_CHECK( glBindTexture( GL_TEXTURE_2D, map->glTextures[ parms->face->texture ] ) );
	}
	else
	{
		GL_CHECK( glBindSampler( 0, map->glLightmapSampler ) ); 
		GL_CHECK( glBindTexture( GL_TEXTURE_2D, map->GetDummyTexture() ) );
	}

	GL_CHECK( glActiveTexture( GL_TEXTURE0 + 1 ) );
	GL_CHECK( glProgramUniform1i( bspProgram, bspProgramUniforms[ "fragLightmapSampler" ], 1 ) );
	GL_CHECK( glBindSampler( 1, map->glLightmapSampler ) );

	if ( parms->face->lightmapIndex >= 0 )
	{
		GL_CHECK( glBindTexture( GL_TEXTURE_2D, map->glLightmaps[ parms->face->lightmapIndex ] ) );
	}
	else
	{
		GL_CHECK( glBindTexture( GL_TEXTURE_2D, map->GetDummyTexture() ) );
	}

	GL_CHECK( glUseProgram( bspProgram ) );
	
	DrawFaceVerts( parms, false );

	GL_CHECK( glUseProgram( 0 ) );
	
	GL_CHECK( glActiveTexture( GL_TEXTURE0 ) );
	GL_CHECK( glBindTexture( GL_TEXTURE_2D, 0 ) );

	GL_CHECK( glActiveTexture( GL_TEXTURE0 + 1 ) );
	GL_CHECK( glBindTexture( GL_TEXTURE_2D, 0 ) );
	
	GL_CHECK( glBindSampler( 0, 0 ) );
	GL_CHECK( glBindSampler( 1, 0 ) );
}

void BSPRenderer::DrawFace( drawFace_t* parms )
{
	GL_CHECK( glBlendFunc( GL_ONE, GL_ZERO ) );
	GL_CHECK( glDepthFunc( GL_LEQUAL ) );

	MapAttribTexCoord( 2, offsetof( bspVertex_t, texCoords[ 0 ] ) );
	MapAttribTexCoord( 3, offsetof( bspVertex_t, texCoords[ 1 ] ) ); 

	// We use textures and effect names as keys into our effectShader map
	if ( ( parms->renderFlags & RENDER_BSP_EFFECT ) && parms->shader )
	{
		if ( parms->shader->hasPolygonOffset )
		{
			SetPolygonOffsetState( true, GLUTIL_POLYGON_OFFSET_FILL | GLUTIL_POLYGON_OFFSET_LINE | GLUTIL_POLYGON_OFFSET_POINT );
		}

		// Each effect pass is allowed only one texture, so we don't need a second texcoord
		GL_CHECK( glDisableVertexAttribArray( 3 ) );

		for ( int i = 0; i < parms->shader->stageCount; ++i )
		{			
			if ( parms->shader->stageBuffer[ i ].isStub )
				continue;

			GL_CHECK( glActiveTexture( GL_TEXTURE0 ) );

			if ( parms->shader->stageBuffer[ i ].mapType == MAP_TYPE_LIGHT_MAP )
			{
				MapAttribTexCoord( 2, offsetof( bspVertex_t, texCoords[ 1 ] ) );
				GL_CHECK( glBindTexture( GL_TEXTURE_2D, map->glLightmaps[ parms->face->lightmapIndex ] ) );
				GL_CHECK( glBindSampler( 0, map->glLightmapSampler ) );
			}
			else 
			{
				MapAttribTexCoord( 2, offsetof( bspVertex_t, texCoords[ 0 ] ) );

				if ( parms->shader->stageBuffer[ i ].mapType == MAP_TYPE_IMAGE ) 
				{
					GL_CHECK( glBindTexture( GL_TEXTURE_2D, parms->shader->stageBuffer[ i ].textureObj ) );
					GL_CHECK( glBindSampler( 0, parms->shader->stageBuffer[ i ].samplerObj ) );
				}
				else
				{
					GL_CHECK( glBindTexture( GL_TEXTURE_2D, map->GetDummyTexture() ) );
					GL_CHECK( glBindSampler( 0, map->glLightmapSampler ) );
				}
			}

			if ( parms->renderFlags & RENDER_BSP_USE_TCMOD )
			{
				if ( parms->shader->stageBuffer[ i ].hasTexMod )
				{
					GL_CHECK( glProgramUniformMatrix2fv( 
							parms->shader->stageBuffer[ i ].programID, 
							parms->shader->stageBuffer[ i ].uniforms.at( "texTransform" ), 1, GL_FALSE, 
							glm::value_ptr( parms->shader->stageBuffer[ i ].texTransform ) ) ); 
				}
				
				if ( parms->shader->stageBuffer[ i ].tcModTurb.enabled )
				{
					GL_CHECK( glProgramUniform1f( 
						parms->shader->stageBuffer[ i ].programID, 
						parms->shader->stageBuffer[ i ].uniforms.at( "tcModTurb" ), 
						DEFORM_CALC_TABLE( 
							deformCache.sinTable, 
							0,										   // base 
							parms->shader->stageBuffer[ i ].tcModTurb.phase,  // offset
						 	glfwGetTime(),
							parms->shader->stageBuffer[ i ].tcModTurb.frequency,
						 	parms->shader->stageBuffer[ i ].tcModTurb.amplitude )
					) );
				}
			}

			GL_CHECK( glBlendFunc( parms->shader->stageBuffer[ i ].rgbSrc, parms->shader->stageBuffer[ i ].rgbDest ) );

			GL_CHECK( glDepthFunc( parms->shader->stageBuffer[ i ].depthFunc ) );

			GL_CHECK( glProgramUniform1i( parms->shader->stageBuffer[ i ].programID, parms->shader->stageBuffer[ i ].uniforms.at( "sampler0" ), 0 ) );
			GL_CHECK( glUseProgram( parms->shader->stageBuffer[ i ].programID ) );

			DrawFaceVerts( parms, true );
		}
	
		if ( parms->shader->hasPolygonOffset )
		{
			SetPolygonOffsetState( false, GLUTIL_POLYGON_OFFSET_FILL | GLUTIL_POLYGON_OFFSET_LINE | GLUTIL_POLYGON_OFFSET_POINT );
		}

		GL_CHECK( glEnableVertexAttribArray( 3 ) );
		GL_CHECK( glBindTexture( GL_TEXTURE_2D, 0 ) );
		GL_CHECK( glBindSampler( 0, 0 ) );
		GL_CHECK( glUseProgram( 0 ) );	
	}
	else
	{
		DrawFaceNoEffect( parms );
	}

	if ( parms->renderFlags & RENDER_BSP_ALWAYS_POLYGON_OFFSET )
	{
		SetPolygonOffsetState( false, GLUTIL_POLYGON_OFFSET_FILL | GLUTIL_POLYGON_OFFSET_LINE | GLUTIL_POLYGON_OFFSET_POINT );
	}

	// Debug information
	if ( parms->renderFlags & RENDER_BSP_LIGHTMAP_INFO )
	{
		ImPrep( parms->pass->view.transform, parms->pass->view.clipTransform );

		uint8_t hasLightmap = FALSE;

		if ( parms->shader )
		{
			const shaderInfo_t& shader = map->effectShaders.at( map->data.textures[ parms->face->texture ].name );
			hasLightmap = shader.hasLightmap;
		}

		GL_CHECK( glPointSize( 10.0f ) );

		// Draw lightmap origin
		glBegin( GL_POINTS );
		if ( hasLightmap )
			glColor3f( 0.0f, 0.0f, 1.0f );
		else
			glColor3f( 0.0f, 1.0f, 0.0f );
		glVertex3f( parms->face->lightmapOrigin.x, parms->face->lightmapOrigin.y, parms->face->lightmapOrigin.z );
		glEnd();

		if ( parms->shader )
		{
			GL_CHECK( glPointSize( 5.0f ) );
			glBegin( GL_POINTS );
			glColor3f( 1.0f, 1.0f, 1.0f );
			for ( uint32_t i = 0; i < map->glFaces[ parms->faceIndex ].indices.size(); ++i )
			{
				const glm::vec3& pos = map->data.vertexes[ map->glFaces[ parms->faceIndex ].indices[ i ] ].position;
				glVertex3f( pos.x, pos.y, pos.z );
			}
			glEnd();
		}

		GL_CHECK( glLoadIdentity() );
	}

    parms->pass->facesRendered[ parms->faceIndex ] = 1;
}

void BSPRenderer::DrawFaceVerts( drawFace_t* parms, bool isEffectPass )
{
	mapModel_t* m = &map->glFaces[ parms->faceIndex ];

	if ( parms->face->type == BSP_FACE_TYPE_POLYGON || parms->face->type == BSP_FACE_TYPE_MESH )
	{
		GL_CHECK( glDrawElements( GL_TRIANGLES, m->indices.size(), GL_UNSIGNED_INT, &m->indices[ 0 ] ) );
	}
	else if ( parms->face->type == BSP_FACE_TYPE_PATCH )
	{
		if ( parms->shader && parms->shader->tessSize != 0.0f )
		{
			DeformVertexes( m, parms );
		}

		LoadBufferLayout( m->vbo, !isEffectPass );		
		
		GL_CHECK( glMultiDrawElements( GL_TRIANGLE_STRIP, 
			&m->trisPerRow[ 0 ], GL_UNSIGNED_INT, ( const GLvoid** ) &m->rowIndices[ 0 ], m->trisPerRow.size() ) );

		LoadBufferLayout( vbo, true );
	}
}

void BSPRenderer::DeformVertexes( mapModel_t* m, drawFace_t* parms )
{
	std::vector< bspVertex_t > verts = m->vertices;
	
	int32_t stride = m->subdivLevel + 1;
	int32_t numPatchVerts = stride * stride;
	int32_t numPatches = verts.size() / numPatchVerts;

	for ( uint32_t i = 0; i < verts.size(); ++i )
	{
		glm::vec3 n( verts[ i ].normal * GenDeformScale( verts[ i ].position, parms->shader ) );

		verts[ i ].position += n;
	}

	UpdateBufferObject< bspVertex_t >( GL_ARRAY_BUFFER, m->vbo, verts );
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
