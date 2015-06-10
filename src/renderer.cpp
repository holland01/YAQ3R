#include "renderer.h"
#include "shader.h"
#include "io.h"
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
	  leaf( nullptr ),
	  lightvol( nullptr ),
	  shader( nullptr ),
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
      vao( 0 ),
      vbo( 0 ),
      deltaTime( 0.0 ),
      frameTime( 0.0f ),
	  curView( VIEW_MAIN )
{
	viewParams_t view;
	view.origin = glm::vec3( -131.291901f, -61.794476f, -163.203659f ); /// debug position which doesn't kill framerate

	camera = new InputCamera( view, EuAng() );
}

BSPRenderer::~BSPRenderer( void )
{
    GL_CHECK( glDeleteVertexArrays( 1, &vao ) );
	GL_CHECK( glDeleteBuffers( 1, &vbo ) );

    delete map;
    delete frustum;
    delete camera;
}

void BSPRenderer::MakeProg( const std::string& name, const std::string& vertPath, const std::string& fragPath,
		const std::vector< std::string >& uniforms, const std::vector< std::string >& attribs, bool bindTransformsUbo )
{
	std::vector< char > vertex, fragment;
	if ( !File_GetBuf( vertex, vertPath ) )
	{
		MLOG_ERROR( "Could not open vertex shader" );
		return;
	}

	if ( !File_GetBuf( fragment, fragPath ) )
	{
		MLOG_ERROR( "Could not open fragment shader" );
		return;
	}

	programs[ name ] = std::unique_ptr< Program >( new Program( vertex, fragment, uniforms, attribs, bindTransformsUbo ) );
}

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

	// Gen transforms UBO
	GL_CHECK( glGenBuffers( 1, &transformBlockObj ) );
	GL_CHECK( glBindBuffer( GL_UNIFORM_BUFFER, transformBlockObj ) );
	GL_CHECK( glBufferData( GL_UNIFORM_BUFFER, transformBlockSize, NULL, GL_STREAM_DRAW ) );
	GL_CHECK( glBufferSubData( GL_UNIFORM_BUFFER, 0, sizeof( glm::mat4 ), glm::value_ptr( camera->ViewData().clipTransform ) ) );
	GL_CHECK( glBindBufferRange( GL_UNIFORM_BUFFER, UBO_TRANSFORMS_BLOCK_BINDING, transformBlockObj, 0, transformBlockSize ) );
	GL_CHECK( glBindBuffer( GL_UNIFORM_BUFFER, 0 ) );

	// Load main shader programs
	{
		
		std::vector< std::string > uniforms = 
		{
			"fragTexSampler",
			"fragLightmapSampler"
		};

		std::vector< std::string > attribs = 
		{
			"position",
			"color",
			"tex0",
			"lightmap"
		};

		std::vector< std::string > sampleunifs( uniforms );
		sampleunifs.push_back( "modelViewProjection" );

		MakeProg( "lightsample", "src/lightsample.vert", "src/main.frag", sampleunifs, attribs, false );
		MakeProg( "main", "src/main.vert", "src/main.frag", uniforms, attribs, true );

		uniforms.insert( uniforms.end(), {
			"fragAmbient",
			"fragDirectional",
			"fragDirToLight"
		} );

		attribs.push_back( "normal" );

		MakeProg( "model", "src/model.vert", "src/model.frag", uniforms, attribs, true );
	}
}

void BSPRenderer::Load( const string& filepath, uint32_t mapLoadFlags )
{
    map->Read( filepath, 1, mapLoadFlags );
	map->WriteLumpToFile( BSP_LUMP_ENTITIES );

	// Allocate vertex data from map and store it all in a single vbo
	GL_CHECK( glBindVertexArray( vao ) );
	GL_CHECK( glBindBuffer( GL_ARRAY_BUFFER, vbo ) );
    GL_CHECK( glBufferData( GL_ARRAY_BUFFER, sizeof( bspVertex_t ) * map->data.numVertexes, map->data.vertexes, GL_STATIC_DRAW ) );

	LoadVertexLayout( true );

	const bspNode_t* root = &map->data.nodes[ 0 ];

	// Base texture setup
	mapDimsLength = ( int ) glm::length( glm::vec3( root->boxMax.x, root->boxMax.y, root->boxMax.z ) );
	lodThreshold = mapDimsLength / 2;

	// Setup light sampler transforms

	GLint viewport[4];
	GL_CHECK( glGetIntegerv( GL_VIEWPORT, viewport ) );
	
	float w = ( float ) viewport[ 2 ];
	float h = ( float ) viewport[ 3 ];

	lightSampler.projection = /*camera->ViewData().clipTransform;*/ glm::ortho< float >( -w, w, -h, h, -10000.0f, 1000000.0f );
	
	const bspModel_t* m = &map->data.models[ 0 ]; 

	const glm::vec3& pos = ( glm::vec3( m->boxMax ) + glm::vec3( m->boxMin ) ) * 0.5f;
	
	lightSampler.view = glm::lookAt( glm::vec3( pos.x, m->boxMin.y, pos.z ), glm::vec3( pos.x, m->boxMax.y, pos.z ), glm::vec3( 0.0f, 1.0f, 1.0f ) );
}

void BSPRenderer::Render( uint32_t renderFlags )
{ 
	double startTime = glfwGetTime();

	const glm::mat4* view = nullptr;
	const glm::mat4* proj = nullptr;

	switch ( curView )
	{
		case VIEW_MAIN:
			view = &camera->ViewData().transform;
			proj = &camera->ViewData().clipTransform;
			break;
		case VIEW_LIGHT_SAMPLE:
			view = &lightSampler.view;
			proj = &/*lightSampler.projection*/camera->ViewData().clipTransform;
			break;
	}

	GL_CHECK( glBindBuffer( GL_UNIFORM_BUFFER, transformBlockObj ) );
	GL_CHECK( glBufferSubData( GL_UNIFORM_BUFFER, 
				0, sizeof( glm::mat4 ), glm::value_ptr( *proj ) ) );
	GL_CHECK( glBufferSubData( GL_UNIFORM_BUFFER, 
		sizeof( glm::mat4 ), sizeof( glm::mat4 ), glm::value_ptr( *view ) ) );

	GL_CHECK( glBindBuffer( GL_UNIFORM_BUFFER, 0 ) );

    drawPass_t pass( map, camera->ViewData() );

    pass.leaf = map->FindClosestLeaf( pass.view.origin );
	pass.renderFlags = renderFlags;
	pass.type = PASS_MAP;
	pass.program = programs[ "main" ].get();

	pass.isSolid = false;
    DrawNode( 0, pass );

	pass.isSolid = true;
	DrawNode( 0, pass );

	DrawFaceList( pass, pass.opaque );
	DrawFaceList( pass, pass.transparent );

	{
		pass.lightvol = &map->data.lightvols[ CalcLightvolIndex( pass ) ];
		pass.program = programs[ "model" ].get();
		pass.type = PASS_MODEL;

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

void BSPRenderer::BindTextureOrDummy( bool predicate, int index, int offset, 
	const Program& program, const std::string& samplerUnif, const std::vector< texture_t >& source )
{
	GL_CHECK( glActiveTexture( GL_TEXTURE0 + offset ) );
	program.LoadInt( samplerUnif, offset );

	if ( predicate )
	{
		const texture_t& tex = source[ index ];
		GL_CHECK( glBindTexture( GL_TEXTURE_2D, tex.handle ) );
		GL_CHECK( glBindSampler( offset, tex.sampler ) );
	}
	else
	{
		GL_CHECK( glBindTexture( GL_TEXTURE_2D, map->GetDummyTexture().handle ) );
		GL_CHECK( glBindSampler( offset, map->GetDummyTexture().sampler ) ); 
	}
}

void BSPRenderer::DrawMapPass( drawPass_t& pass )
{
	BindTextureOrDummy( map->glTextures[ pass.face->texture ].handle != 0, 
		pass.face->texture, 0, *( pass.program ), "fragTexSampler", map->glTextures );

	BindTextureOrDummy( pass.face->lightmapIndex >= 0, 
		pass.face->lightmapIndex, 1, *( pass.program ), "fragLightmapSampler", map->glLightmaps );
	
	pass.program->Bind();
	
	DrawFaceVerts( pass, false );

	GL_CHECK( glUseProgram( 0 ) );

	pass.program->Release();
	
	GL_CHECK( glActiveTexture( GL_TEXTURE0 ) );
	GL_CHECK( glBindTexture( GL_TEXTURE_2D, 0 ) );

	GL_CHECK( glActiveTexture( GL_TEXTURE0 + 1 ) );
	GL_CHECK( glBindTexture( GL_TEXTURE_2D, 0 ) );
	
	GL_CHECK( glBindSampler( 0, 0 ) );
	GL_CHECK( glBindSampler( 1, 0 ) );
}

void BSPRenderer::DrawEffectPass( drawPass_t& pass )
{
	if ( pass.shader->hasPolygonOffset )
	{
		SetPolygonOffsetState( true, GLUTIL_POLYGON_OFFSET_FILL | GLUTIL_POLYGON_OFFSET_LINE | GLUTIL_POLYGON_OFFSET_POINT );
	}

	// Each effect pass is allowed only one texture, so we don't need a second texcoord
	GL_CHECK( glDisableVertexAttribArray( 3 ) );

	for ( int i = 0; i < pass.shader->stageCount; ++i )
	{			
		const shaderStage_t& stage = pass.shader->stageBuffer[ i ];

		if ( stage.isStub )
			continue;

		if ( Shade_IsIdentColor( stage ) )
		{
			GL_CHECK( glDisableVertexAttribArray( 1 ) );
		}

		GL_CHECK( glBlendFunc( stage.rgbSrc, stage.rgbDest ) );
		GL_CHECK( glDepthFunc( stage.depthFunc ) );	

		GL_CHECK( glActiveTexture( GL_TEXTURE0 ) );
			
		if ( stage.mapType == MAP_TYPE_LIGHT_MAP )
		{
			const texture_t& lightmap = map->glLightmaps[ pass.face->lightmapIndex ];

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
			stage.program->LoadMat2( "texTransform", stage.texTransform );
		}
				
		if ( stage.tcModTurb.enabled )
		{
			float turb = DEFORM_CALC_TABLE( 
					deformCache.sinTable, 
					0,
					stage.tcModTurb.phase,
					glfwGetTime(),
					stage.tcModTurb.frequency,
					stage.tcModTurb.amplitude );

			stage.program->LoadFloat( "tcModTurb", turb );
		}

		if ( stage.tcModScroll.enabled )
		{
			stage.program->LoadVec4( "tcModScroll", stage.tcModScroll.speed );
		}

		stage.program->LoadInt( "sampler0", 0 );
		
		stage.program->Bind();
		DrawFaceVerts( pass, true );
		stage.program->Release();

		if ( Shade_IsIdentColor( stage ) )
		{
			GL_CHECK( glEnableVertexAttribArray( 1 ) );
		}
	}
	
	if ( pass.shader->hasPolygonOffset )
	{
		SetPolygonOffsetState( false, GLUTIL_POLYGON_OFFSET_FILL | GLUTIL_POLYGON_OFFSET_LINE | GLUTIL_POLYGON_OFFSET_POINT );
	}

	GL_CHECK( glEnableVertexAttribArray( 3 ) );
	GL_CHECK( glBindTexture( GL_TEXTURE_2D, 0 ) );
	GL_CHECK( glBindSampler( 0, 0 ) );
}

void BSPRenderer::DrawDebugInfo( drawPass_t& pass )
{
	ImPrep( pass.view.transform, pass.view.clipTransform );

	uint8_t hasLightmap = FALSE;

	if ( pass.shader )
	{
		const shaderInfo_t& shader = map->effectShaders.at( map->data.textures[ pass.face->texture ].name );
		hasLightmap = shader.hasLightmap;
	}

	GL_CHECK( glPointSize( 10.0f ) );

	// Draw lightmap origin
	glBegin( GL_POINTS );
	if ( hasLightmap )
		glColor3f( 0.0f, 0.0f, 1.0f );
	else
		glColor3f( 0.0f, 1.0f, 0.0f );
	glVertex3f( pass.face->lightmapOrigin.x, pass.face->lightmapOrigin.y, pass.face->lightmapOrigin.z );
	glEnd();

	if ( pass.shader )
	{
		GL_CHECK( glPointSize( 5.0f ) );
		glBegin( GL_POINTS );
		glColor3f( 1.0f, 1.0f, 1.0f );
		for ( uint32_t i = 0; i < map->glFaces[ pass.faceIndex ].indices.size(); ++i )
		{
			const glm::vec3& pos = map->data.vertexes[ map->glFaces[ pass.faceIndex ].indices[ i ] ].position;
			glVertex3f( pos.x, pos.y, pos.z );
		}
		glEnd();
	}

	GL_CHECK( glLoadIdentity() );
}

void BSPRenderer::DrawFace( drawPass_t& pass )
{
	GL_CHECK( glBlendFunc( GL_ONE, GL_ZERO ) );
	GL_CHECK( glDepthFunc( GL_LEQUAL ) );

	MapAttribTexCoord( 2, offsetof( bspVertex_t, texCoords[ 0 ] ) );
	MapAttribTexCoord( 3, offsetof( bspVertex_t, texCoords[ 1 ] ) ); 

	if ( pass.shader )
	{
		DrawEffectPass( pass );
	}
	else
	{
		DrawMapPass( pass );
	}

	// Debug information
	if ( pass.renderFlags & RENDER_BSP_LIGHTMAP_INFO )
	{
		DrawDebugInfo( pass );
	}

    pass.facesVisited[ pass.faceIndex ] = 1;
}

void BSPRenderer::DrawFaceVerts( drawPass_t& pass, bool isEffectPass )
{
	mapModel_t* m = &map->glFaces[ pass.faceIndex ];

	if ( pass.lightvol && pass.type == PASS_MODEL )
	{
		float phi = glm::radians( ( float )pass.lightvol->direction.x * 4.0f ); 
		float theta = glm::radians( ( float )pass.lightvol->direction.y * 4.0f );
		
		glm::vec3 dirToLight( glm::cos( theta ) * glm::cos( phi ), glm::sin( phi ), glm::cos( phi ) * glm::sin( theta ) );
		
		glm::vec3 ambient( pass.lightvol->ambient );
		ambient *= Inv255< float >();

		glm::vec3 directional( pass.lightvol->directional );
		directional *= Inv255< float >();

		pass.program->LoadVec3( "fragDirToLight", dirToLight );
		pass.program->LoadVec3( "fragAmbient", ambient );
		pass.program->LoadVec3( "fragDirectional", directional );
	}

	if ( pass.face->type == BSP_FACE_TYPE_POLYGON || pass.face->type == BSP_FACE_TYPE_MESH )
	{
		GL_CHECK( glDrawElements( GL_TRIANGLES, m->indices.size(), GL_UNSIGNED_INT, &m->indices[ 0 ] ) );
	}
	else if ( pass.face->type == BSP_FACE_TYPE_PATCH )
	{
		if ( pass.shader && pass.shader->tessSize != 0.0f )
		{
			DeformVertexes( m, pass );
		}

		LoadBufferLayout( m->vbo, !isEffectPass );		
		
		GL_CHECK( glMultiDrawElements( GL_TRIANGLE_STRIP, 
			&m->trisPerRow[ 0 ], GL_UNSIGNED_INT, ( const GLvoid** ) &m->rowIndices[ 0 ], m->trisPerRow.size() ) );

		LoadBufferLayout( vbo, true );
	}
}

void BSPRenderer::DeformVertexes( mapModel_t* m, drawPass_t& pass )
{
	std::vector< bspVertex_t > verts = m->vertices;
	
	int32_t stride = m->subdivLevel + 1;
	int32_t numPatchVerts = stride * stride;
	int32_t numPatches = verts.size() / numPatchVerts;

	for ( uint32_t i = 0; i < verts.size(); ++i )
	{
		glm::vec3 n( verts[ i ].normal * GenDeformScale( verts[ i ].position, pass.shader ) );
		verts[ i ].position += n;
	}

	UpdateBufferObject< bspVertex_t >( GL_ARRAY_BUFFER, m->vbo, verts );
}

int BSPRenderer::CalcLightvolIndex( const drawPass_t& pass ) const
{
	const glm::vec3& max = map->data.models[ 0 ].boxMax;
	const glm::vec3& min = map->data.models[ 0 ].boxMin;
		 
	glm::vec3 input;
	input.x = glm::abs( glm::floor( pass.view.origin.x * Inv64< float >() ) - glm::ceil( min.x * Inv64< float >() ) ); 
	input.y = glm::abs( glm::floor( pass.view.origin.y * Inv64< float >() ) - glm::ceil( min.y * Inv64< float >() ) ); 
	input.z = glm::abs( glm::floor( pass.view.origin.z * Inv128< float >() ) - glm::ceil( min.z * Inv128< float >() ) ); 
		
	glm::vec3 interp = input / map->lightvolGrid;

	glm::ivec3 dindex;
	dindex.x = static_cast< int >( interp.x * map->lightvolGrid.x );
	dindex.y = static_cast< int >( interp.y * map->lightvolGrid.y );
	dindex.z = static_cast< int >( interp.z * map->lightvolGrid.z );
	
	// Performs an implicit cast from a vec3 to ivec3
	glm::ivec3 dims( map->lightvolGrid );

	return ( dindex.z * dims.x * dims.y + dims.x * dindex.y + dindex.x ) % map->data.numLightvols;
	//printf( "index: %i / %i, interp: %s, dindex: %s \r\n", lvIndex, map->data.numLightvols, glm::to_string( interp ).c_str(), glm::to_string( dindex ).c_str() ); 
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
