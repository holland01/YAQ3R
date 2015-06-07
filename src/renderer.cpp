#include "renderer.h"
#include "shader.h"
#include "log.h"
#include "math_util.h"
#include "effect_shader.h"
#include "deform.h"
#include <glm/gtx/string_cast.hpp>

using namespace std;

drawPass_t::drawPass_t( const Q3BspMap* const& map, const viewParams_t& viewData )
    : isSolid( true ),
	  faceIndex( 0 ), renderFlags( 0 ),
	  brush( nullptr ),
	  face( nullptr ),
	  shader( nullptr ),
	  leaf( nullptr ),
	  view( viewData )
{
    facesVisited.resize( map->data.numFaces, 0 );
}

drawPass_t::~drawPass_t( void )
{
}


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

	GL_CHECK( glClearColor( 1.0f, 1.0f, 1.0f, 1.0f ) );

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
	GL_CHECK( glBindBufferRange( GL_UNIFORM_BUFFER, UBO_TRANSFORMS_BLOCK_BINDING, transformBlockObj, 0, transformBlockSize ) );
	GL_CHECK( glBindBuffer( GL_UNIFORM_BUFFER, 0 ) );

	MapProgramToUBO( bspProgram, "Transforms" );
}

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
	GL_CHECK( glBindBuffer( GL_UNIFORM_BUFFER, 0 ) );

    drawPass_t pass( map, camera->ViewData() );

    pass.leaf = map->FindClosestLeaf( pass.view.origin );
	pass.renderFlags = renderFlags;

	pass.isSolid = false;
    DrawNode( 0, pass );

	pass.isSolid = true;
	DrawNode( 0, pass );

	DrawFaceList( pass, pass.opaque );
	DrawFaceList( pass, pass.transparent );

	{
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
				if ( pass.facesVisited[ model->faceOffset + j ] )
					continue;

				pass.faceIndex = model->faceOffset + j;
				pass.face = &map->data.faces[ pass.faceIndex ];
				pass.shader = map->GetShaderInfo( pass.faceIndex );

				DrawFace( pass );
			}
		}
	}
	
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

void BSPRenderer::DrawNode( int nodeIndex, drawPass_t& pass )
{
    if ( nodeIndex < 0 )
    {
        const bspLeaf_t* viewLeaf = &map->data.leaves[ -( nodeIndex + 1 ) ];

        if ( !map->IsClusterVisible( pass.leaf->clusterIndex, viewLeaf->clusterIndex ) )
            return;

		AABB leafBounds;
		leafBounds.maxPoint = glm::vec3( viewLeaf->boxMax.x, viewLeaf->boxMax.y, viewLeaf->boxMax.z );
		leafBounds.minPoint = glm::vec3( viewLeaf->boxMin.x, viewLeaf->boxMin.y, viewLeaf->boxMin.z );

        if ( !frustum->IntersectsBox( leafBounds ) )
            return;

        for ( int i = 0; i < viewLeaf->numLeafFaces; ++i )
        {
            int faceIndex = map->data.leafFaces[ viewLeaf->leafFaceOffset + i ].index;

            if ( pass.facesVisited[ faceIndex ] )
                continue;
			
			// if pass.facesVisited[ faceIndex ] is still false after this
			// evaluation, we'll pick it up on the next pass as it will meet
			// the necessary criteria then.
			if ( !pass.isSolid )
			{
				if ( map->IsTransFace( faceIndex ) )
				{
					pass.transparent.push_back( faceIndex );
					pass.facesVisited[ faceIndex ] = true;
				}
			}
			else
			{
				pass.opaque.push_back( faceIndex );
				pass.facesVisited[ faceIndex ] = true;
			}
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
        if ( pass.isSolid == ( d > plane->distance ) )
        {
            DrawNode( node->children[ 0 ], pass );
			DrawNode( node->children[ 1 ], pass );
		}
        else
        {
            DrawNode( node->children[ 1 ], pass );
            DrawNode( node->children[ 0 ], pass );
        }
    }
}

void BSPRenderer::DrawFaceNoEffect( drawPass_t& parms )
{
	GL_CHECK( glActiveTexture( GL_TEXTURE0 ) );
	GL_CHECK( glProgramUniform1i( bspProgram, bspProgramUniforms[ "fragTexSampler" ], 0 ) );

	if ( map->glTextures[ parms.face->texture ].handle )
	{
		const texture_t& tex = map->glTextures[ parms.face->texture ];
		GL_CHECK( glBindTexture( GL_TEXTURE_2D, tex.handle ) );
		GL_CHECK( glBindSampler( 0, tex.sampler ) );
	}
	else
	{
		GL_CHECK( glBindTexture( GL_TEXTURE_2D, map->GetDummyTexture().handle ) );
		GL_CHECK( glBindSampler( 0, map->GetDummyTexture().sampler ) ); 
	}

	GL_CHECK( glActiveTexture( GL_TEXTURE0 + 1 ) );
	GL_CHECK( glProgramUniform1i( bspProgram, bspProgramUniforms[ "fragLightmapSampler" ], 1 ) );

	if ( parms.face->lightmapIndex >= 0 )
	{
		const texture_t& lightmap = map->glLightmaps[ parms.face->lightmapIndex ];

		GL_CHECK( glBindTexture( GL_TEXTURE_2D, lightmap.handle ) );
		GL_CHECK( glBindSampler( 1, lightmap.sampler ) );
	}
	else
	{
		GL_CHECK( glBindTexture( GL_TEXTURE_2D, map->GetDummyTexture().handle ) );
		GL_CHECK( glBindSampler( 1, map->GetDummyTexture().sampler ) );
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

void BSPRenderer::DrawFace( drawPass_t& parms )
{
	GL_CHECK( glBlendFunc( GL_ONE, GL_ZERO ) );
	GL_CHECK( glDepthFunc( GL_LEQUAL ) );

	MapAttribTexCoord( 2, offsetof( bspVertex_t, texCoords[ 0 ] ) );
	MapAttribTexCoord( 3, offsetof( bspVertex_t, texCoords[ 1 ] ) ); 

	// We use textures and effect names as keys into our effectShader map
	if ( ( parms.renderFlags & RENDER_BSP_EFFECT ) && parms.shader )
	{
		if ( parms.shader->hasPolygonOffset )
		{
			SetPolygonOffsetState( true, GLUTIL_POLYGON_OFFSET_FILL | GLUTIL_POLYGON_OFFSET_LINE | GLUTIL_POLYGON_OFFSET_POINT );
		}

		// Each effect pass is allowed only one texture, so we don't need a second texcoord
		GL_CHECK( glDisableVertexAttribArray( 3 ) );

		for ( int i = 0; i < parms.shader->stageCount; ++i )
		{			
			const shaderStage_t& stage = parms.shader->stageBuffer[ i ];

			if ( stage.isStub )
				continue;

			if ( Shade_IsIdentColor( stage ) )
			{
				GL_CHECK( glDisableVertexAttribArray( 1 ) );
			}

			GL_CHECK( glActiveTexture( GL_TEXTURE0 ) );
			
			if ( stage.mapType == MAP_TYPE_LIGHT_MAP )
			{
				const texture_t& lightmap = map->glLightmaps[ parms.face->lightmapIndex ];

				MapAttribTexCoord( 2, offsetof( bspVertex_t, texCoords[ 1 ] ) );
				GL_CHECK( glBindTexture( GL_TEXTURE_2D, lightmap.handle ) );
				GL_CHECK( glBindSampler( 0, lightmap.sampler ) );
			}
			else 
			{
				MapAttribTexCoord( 2, offsetof( bspVertex_t, texCoords[ 0 ] ) );

				if ( stage.mapType == MAP_TYPE_IMAGE ) 
				{
					GL_CHECK( glBindTexture( GL_TEXTURE_2D, stage.texture.handle ) );
					GL_CHECK( glBindSampler( 0, stage.texture.sampler ) );
				}
				else
				{
					GL_CHECK( glBindTexture( GL_TEXTURE_2D, map->GetDummyTexture().handle ) );
					GL_CHECK( glBindSampler( 0, map->GetDummyTexture().sampler ) );
				}
			}

			if ( stage.hasTexMod )
			{
				GL_CHECK( glProgramUniformMatrix2fv( 
						stage.programID, 
						stage.uniforms.at( "texTransform" ), 1, GL_FALSE, 
						glm::value_ptr( stage.texTransform ) ) ); 
			}
				
			if ( stage.tcModTurb.enabled )
			{
				GL_CHECK( glProgramUniform1f( 
					stage.programID, 
					stage.uniforms.at( "tcModTurb" ), 
					DEFORM_CALC_TABLE( 
						deformCache.sinTable, 
						0,
						stage.tcModTurb.phase,
						glfwGetTime(),
						stage.tcModTurb.frequency,
						stage.tcModTurb.amplitude )
				) );
			}

			if ( stage.tcModScroll.enabled )
			{
				GL_CHECK( glProgramUniform4fv( 
					stage.programID, stage.uniforms.at( "tcModScroll" ), 1, stage.tcModScroll.speed ) );
			}

			GL_CHECK( glBlendFunc( stage.rgbSrc, stage.rgbDest ) );
			GL_CHECK( glDepthFunc( stage.depthFunc ) );
			GL_CHECK( glProgramUniform1i( stage.programID, stage.uniforms.at( "sampler0" ), 0 ) );
			GL_CHECK( glUseProgram( stage.programID ) );

			DrawFaceVerts( parms, true );

			if ( Shade_IsIdentColor( stage ) )
			{
				GL_CHECK( glEnableVertexAttribArray( 1 ) );
			}
		}
	
		if ( parms.shader->hasPolygonOffset )
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

	if ( parms.renderFlags & RENDER_BSP_ALWAYS_POLYGON_OFFSET )
	{
		SetPolygonOffsetState( false, GLUTIL_POLYGON_OFFSET_FILL | GLUTIL_POLYGON_OFFSET_LINE | GLUTIL_POLYGON_OFFSET_POINT );
	}

	// Debug information
	if ( parms.renderFlags & RENDER_BSP_LIGHTMAP_INFO )
	{
		ImPrep( parms.view.transform, parms.view.clipTransform );

		uint8_t hasLightmap = FALSE;

		if ( parms.shader )
		{
			const shaderInfo_t& shader = map->effectShaders.at( map->data.textures[ parms.face->texture ].name );
			hasLightmap = shader.hasLightmap;
		}

		GL_CHECK( glPointSize( 10.0f ) );

		// Draw lightmap origin
		glBegin( GL_POINTS );
		if ( hasLightmap )
			glColor3f( 0.0f, 0.0f, 1.0f );
		else
			glColor3f( 0.0f, 1.0f, 0.0f );
		glVertex3f( parms.face->lightmapOrigin.x, parms.face->lightmapOrigin.y, parms.face->lightmapOrigin.z );
		glEnd();

		if ( parms.shader )
		{
			GL_CHECK( glPointSize( 5.0f ) );
			glBegin( GL_POINTS );
			glColor3f( 1.0f, 1.0f, 1.0f );
			for ( uint32_t i = 0; i < map->glFaces[ parms.faceIndex ].indices.size(); ++i )
			{
				const glm::vec3& pos = map->data.vertexes[ map->glFaces[ parms.faceIndex ].indices[ i ] ].position;
				glVertex3f( pos.x, pos.y, pos.z );
			}
			glEnd();
		}

		GL_CHECK( glLoadIdentity() );
	}

    parms.facesVisited[ parms.faceIndex ] = 1;
}

void BSPRenderer::DrawFaceVerts( drawPass_t& parms, bool isEffectPass )
{
	mapModel_t* m = &map->glFaces[ parms.faceIndex ];

	if ( parms.face->type == BSP_FACE_TYPE_POLYGON || parms.face->type == BSP_FACE_TYPE_MESH )
	{
		GL_CHECK( glDrawElements( GL_TRIANGLES, m->indices.size(), GL_UNSIGNED_INT, &m->indices[ 0 ] ) );
	}
	else if ( parms.face->type == BSP_FACE_TYPE_PATCH )
	{
		if ( parms.shader && parms.shader->tessSize != 0.0f )
		{
			DeformVertexes( m, parms );
		}

		LoadBufferLayout( m->vbo, !isEffectPass );		
		
		GL_CHECK( glMultiDrawElements( GL_TRIANGLE_STRIP, 
			&m->trisPerRow[ 0 ], GL_UNSIGNED_INT, ( const GLvoid** ) &m->rowIndices[ 0 ], m->trisPerRow.size() ) );

		LoadBufferLayout( vbo, true );
	}
}

void BSPRenderer::DeformVertexes( mapModel_t* m, drawPass_t& parms )
{
	std::vector< bspVertex_t > verts = m->vertices;
	
	int32_t stride = m->subdivLevel + 1;
	int32_t numPatchVerts = stride * stride;
	int32_t numPatches = verts.size() / numPatchVerts;

	for ( uint32_t i = 0; i < verts.size(); ++i )
	{
		glm::vec3 n( verts[ i ].normal * GenDeformScale( verts[ i ].position, parms.shader ) );

		verts[ i ].position += n;
	}

	UpdateBufferObject< bspVertex_t >( GL_ARRAY_BUFFER, m->vbo, verts );
}

int BSPRenderer::CalcSubdivision( const drawPass_t& pass, const AABB& bounds )
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
