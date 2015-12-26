#include "renderer.h"
#include "shader.h"
#include "io.h"
#include "math_util.h"
#include "effect_shader.h"
#include "deform.h"
#include <glm/gtx/string_cast.hpp>

struct config_t
{
	bool drawFacesOnly: 1;
	bool drawIrradiance: 1;
	bool drawFacePatches: 1;
	bool drawFaceBounds: 1;
};

static config_t config = 
{
    false,
	false,
	false,
    false
};

static uint64_t frameCount = 0;

static INLINE void AddSurfaceData( drawSurface_t& surf, int faceIndex, std::vector< mapModel_t >& glFaces )
{
	mapModel_t& model = glFaces[ faceIndex ];

	if ( surf.faceType == BSP_FACE_TYPE_PATCH )
	{
		surf.indexBuffers.insert( surf.indexBuffers.end(), model.rowIndices.begin(), model.rowIndices.end() );
		surf.indexBufferSizes.insert( surf.indexBufferSizes.end(), model.trisPerRow.begin(), model.trisPerRow.end() );
	}
	else
	{
		surf.indexBuffers.push_back( &model.indices[ 0 ] );
		surf.indexBufferSizes.push_back( model.indices.size() );
	}

	if ( surf.shader 
    && ( !!( surf.shader->surfaceParms & SURFPARM_ENVMAP ) || surf.shader->deform ) )
	{
		surf.faceIndices.push_back( faceIndex );
	}
}

//--------------------------------------------------------------
mapModel_t::mapModel_t( void )
	: deform( false ),
	  vboOffset( 0 ),
	  subdivLevel( 0 ),
	  envmap( nullptr )
{
}

mapModel_t::~mapModel_t( void )
{
}

void mapModel_t::CalcBounds( int32_t faceType, const mapData_t& data )
{
	bounds.Empty();

    auto LCheck = [ & ]( const glm::vec3& v )
	{
		if ( v.x < bounds.minPoint.x ) bounds.minPoint.x = v.x;
		if ( v.y < bounds.minPoint.y ) bounds.minPoint.y = v.y;
		if ( v.z > bounds.minPoint.z ) bounds.minPoint.z = v.z;

		if ( v.x > bounds.maxPoint.x ) bounds.maxPoint.x = v.x;
		if ( v.y > bounds.maxPoint.y ) bounds.maxPoint.y = v.y;
		if ( v.y < bounds.maxPoint.z ) bounds.maxPoint.z = v.z;
	};

	if ( faceType == BSP_FACE_TYPE_PATCH )
	{
        for ( const bspVertex_t& v: patchVertices )
		{
            LCheck( v.position );
		}
	}
	else
	{
		for ( size_t i = 0; i < indices.size(); ++i ) 
		{
            LCheck( data.vertexes[ indices[ i ] ].position );
		}
	}
}

//--------------------------------------------------------------
drawPass_t::drawPass_t( const Q3BspMap* const& map, const viewParams_t& viewData )
    : isSolid( true ),
	  envmap( false ),
	  faceIndex( 0 ), viewLeafIndex( 0 ), 
	  type( PASS_DRAW ), drawType( PASS_DRAW_MAIN ),
	  renderFlags( 0 ),
	  face( nullptr ),
	  leaf( nullptr ),
	  lightvol( nullptr ),
	  shader( nullptr ),
	  view( viewData )
{
    facesVisited.resize( map->data.numFaces, 0 );
}

//--------------------------------------------------------------
BSPRenderer::BSPRenderer( float viewWidth, float viewHeight )
	:	glEffects( {
			{ 
				"tcModTurb", 
				[]( const Program& p, const effect_t& e ) -> void
				{
					float turb = DEFORM_CALC_TABLE( 
					deformCache.sinTable, 
					0,
					e.data.wave.phase,
					glfwGetTime(),
					e.data.wave.frequency,
					e.data.wave.amplitude );
					p.LoadFloat( "tcModTurb", turb );					
				} 
			},
			{
				"tcModScale",
				[]( const Program& p, const effect_t& e ) -> void
				{
					p.LoadMat2( "tcModScale", &e.data.scale2D[ 0 ][ 0 ] );
				}
			},
			{
				"tcModScroll",
				[]( const Program& p, const effect_t& e ) -> void
				{
					p.LoadVec4( "tcModScroll", e.data.xyzw );
				}
			},
			{
				"tcModRotate",
				[]( const Program& p, const effect_t& e ) -> void
				{
					p.LoadMat2( "texRotate", &e.data.rotation2D.transform[ 0 ][ 0 ] );
					p.LoadVec2( "texCenter", e.data.rotation2D.center );
				}
			}
		} ),
        currLeaf( nullptr ),
        vao( 0 ),
        vbo( 0 ),
        deltaTime( 0.0f ),
        frameTime( 0.0f ),
        map ( new Q3BspMap() ),
        camera( nullptr ),
        frustum( new Frustum() ),
		transformBlockIndex( 0 ),
		transformBlockObj( 0 ),
		transformBlockSize( sizeof( glm::mat4 ) * 2 ),
		curView( VIEW_MAIN )
{
	viewParams_t view;
	view.origin = glm::vec3( -131.291901f, -61.794476f, -163.203659f ); /// debug position which doesn't kill framerate

	camera = new InputCamera( view, EuAng() );
	camera->SetPerspective( 45.0f, viewWidth, viewHeight, 0.1f, 5000.0f );
}

BSPRenderer::~BSPRenderer( void )
{
    GL_CHECK( glDeleteVertexArrays( 1, &vao ) );
	DeleteBufferObject( GL_ARRAY_BUFFER, vbo );

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

	glPrograms[ name ] = std::unique_ptr< Program >( new Program( vertex, fragment, uniforms, attribs, bindTransformsUbo ) );
}

void BSPRenderer::Prep( void )
{
	GL_CHECK( glEnable( GL_DEPTH_TEST ) );
	GL_CHECK( glEnable( GL_BLEND ) );
	GL_CHECK( glEnable( GL_FRAMEBUFFER_SRGB ) );

    GL_CHECK( glDepthFunc( GL_LEQUAL ) );
	GL_CHECK( glBlendEquationSeparate( GL_FUNC_ADD, GL_FUNC_ADD ) );
	
	GL_CHECK( glPointSize( 20.0f ) );
	//GL_CHECK( glPolygonOffset( 5.0f, 1.0f ) );

	GL_CHECK( glClearColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
	GL_CHECK( glClearDepth( 1.0f ) );

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

	glDummyBiases.fill( glm::vec2( 1.0f ) );

	// Load main shader glPrograms
	{
		std::vector< std::string > attribs = 
		{
			"position",
			"color",
			"lightmap",
			"tex0"
		};

		std::vector< std::string > uniforms = 
		{
			"fragSampler",
			"fragLightmapSampler",
			"fragBiases"
		};

		/*
		{
			std::vector< std::string > fragTexBiasNames = Program::ArrayLocationNames( "fragTexBiases", GLConfig::MAX_MIP_LEVELS );
			uniforms.insert( uniforms.end(), fragTexBiasNames.begin(), fragTexBiasNames.end() );

			std::vector< std::string > fragLightmapBiasNames = Program::ArrayLocationNames( "fragLightmapBiases", GLConfig::MAX_MIP_LEVELS );
			uniforms.insert( uniforms.end(), fragLightmapBiasNames.begin(), fragLightmapBiasNames.end() );
		}
		*/

		MakeProg( "main", "src/main.vert", "src/main.frag", uniforms, attribs, true );

		/*
		uniforms.insert( uniforms.end(), {
			"fragAmbient",
			"fragDirectional",
			"fragDirToLight"
		} );

		attribs.push_back( "normal" );

		MakeProg( "model", "src/model.vert", "src/model.frag", uniforms, attribs, true );
		*/

		uniforms = 
		{
			"fragRadianceSampler",
			"fragTargetPlane",
			"fragMin",
			"fragMax"
		};

		attribs = 
		{
			"position",
			"normal"
		};

		MakeProg( "irradiate", "src/irradiate.vert", "src/irradiate.frag", uniforms, attribs, true );

		MakeProg( "debug", "src/debug.vert", "src/debug.frag", { "fragColor" }, { "position" }, true );
	}
}

bool BSPRenderer::IsTransFace( int32_t faceIndex, const shaderInfo_t* shader ) const
{
	const bspFace_t* face = &map->data.faces[ faceIndex ];

	if ( face->texture != -1 )
	{
		if ( shader )
		{
			return ( shader->surfaceParms & SURFPARM_TRANS ) != 0;
		}
		else
		{
			return glTextures[ face->texture ].bpp == 4;
		}
	}

	return false;
}

void BSPRenderer::LoadPassParams( drawPass_t& p, int32_t face, passDrawType_t defaultPass ) const
{
	p.face = &map->data.faces[ face ];
	p.faceIndex = face;
	p.shader = map->GetShaderInfo( face );

	if ( p.shader )
	{
		p.drawType = PASS_DRAW_EFFECT;
	}
	else
	{
		p.drawType = defaultPass;
	}
}

void BSPRenderer::LoadTextureArray( std::unique_ptr< textureArray_t >& texArray, 
    std::vector< gImageParams_t >& images, int32_t width, int32_t height )
{
    images.push_back( glDummyTexture );

    texArray.reset( new textureArray_t( width, height, images.size(), false ) );
    for ( uint32_t i = 0; i < images.size(); ++i )
	{
        const gImageParams_t& img = images[ i ];
        if ( !img.data.empty() )
		{
            /*
            GLuint sampler = img.sampler;
			if ( !sampler )
			{
                sampler = GenSampler( img.mipmap, img.wrap );
			}
            */

            GLuint sampler = GenSampler( img.mipmap, img.wrap );

            glm::ivec3 dims( img.width, img.height, i );
            texArray->LoadSlice( sampler, dims, img.data, false );
		}
	}
}

static glm::vec3 CalcUpVector( const glm::vec3& forward, const glm::vec3& desired )
{
	float cosAng = glm::abs( glm::dot( glm::normalize( forward ), desired ) );
	if ( cosAng == 1.0f )
	{
		return glm::normalize( glm::cross( glm::vec3( 1.0f, 0.0f, 0.0f ), forward ) ); 
	}
	else
	{
		return desired;
	}
}

void BSPRenderer::Load( const std::string& filepath, uint32_t mapLoadFlags )
{
    map->Read( filepath, 1, mapLoadFlags );
	map->WriteLumpToFile( BSP_LUMP_ENTITIES );

    std::vector< gImageParams_t > shaderTextures;
    glm::ivec2 shaderMegaDims = S_LoadShaders( &map->data, shaderTextures, map->effectShaders, mapLoadFlags );
	
    GSetImageBuffer( glDummyTexture, 64, 64, 4, 255 );

    theTexture = GMakeTexture( shaderTextures, 0 );

    LoadTextureArray( glShaderArray, shaderTextures, shaderMegaDims.x, shaderMegaDims.y );

	//---------------------------------------------------------------------
	// Load Textures:
	// This is just a temporary hack to brute force load assets without taking into account the effect shader files.
	// Now, we find and generate the textures. We first start with the image files.
	//---------------------------------------------------------------------

	GLint oldAlign;
	GL_CHECK( glGetIntegerv( GL_UNPACK_ALIGNMENT, &oldAlign ) );
	GL_CHECK( glPixelStorei( GL_UNPACK_ALIGNMENT, 1 ) );

	glTextures.resize( map->data.numTextures );
    static const char* validImgExt[] =
	{
		".jpg", ".png", ".tga", ".tiff", ".bmp"
	};

	GLsizei width = 0, height = 0;

	for ( int32_t t = 0; t < map->data.numTextures; t++ )
	{
		bool success = false;

		std::string fname( map->data.textures[ t ].name );

		const std::string& texPath = map->data.basePath + fname;
		
		// If we don't have a file extension appended in the name,
		// try to find one for it which is valid
		if ( fname.find_last_of( '.' ) == std::string::npos )
		{
			glTextures[ t ].wrap = GL_REPEAT;

			for ( int32_t i = 0; i < SIGNED_LEN( validImgExt ); ++i )
			{
				const std::string& str = texPath + std::string( validImgExt[ i ] );

                if ( GLoadImageFromFile( str, glTextures[ t ] ) )
				{
					width = glm::max( width, glTextures[ t ].width );
					height = glm::max( height, glTextures[ t ].height );
					success = true;
					
					glTextures[ t ].wrap = GL_REPEAT;
                    glTextures[ t ].minFilter = GL_LINEAR;
                    //glTextures[ t ].minFilter = GL_LINEAR_MIPMAP_LINEAR;
					
					break;
				}
			}
		}
		
		// Stub out the texture for this iteration by continue; warn user
		if ( !success )
		{
			goto FAIL_WARN;
		}

		continue;

FAIL_WARN:
		MLOG_WARNING( "Could not find a file extension for \'%s\'", texPath.c_str() );
	}

	LoadTextureArray( glTextureArray, glTextures, width, height );

	// And then generate all of the lightmaps
	glLightmaps.resize( map->data.numLightmaps );
	for ( int32_t l = 0; l < map->data.numLightmaps; ++l )
	{	
        GSetImageBuffer( glLightmaps[ l ], BSP_LIGHTMAP_WIDTH, BSP_LIGHTMAP_HEIGHT, 4, 255 );
		
        Pixels_24BitTo32Bit( &glLightmaps[ l ].data[ 0 ],
			&map->data.lightmaps[ l ].map[ 0 ][ 0 ][ 0 ], BSP_LIGHTMAP_WIDTH * BSP_LIGHTMAP_HEIGHT );

		glLightmaps[ l ].wrap = GL_REPEAT;
        glLightmaps[ l ].minFilter = GL_LINEAR; //GL_LINEAR_MIPMAP_LINEAR;
	}
	LoadTextureArray( glLightmapArray, glLightmaps, BSP_LIGHTMAP_WIDTH, BSP_LIGHTMAP_HEIGHT );

	GL_CHECK( glPixelStorei( GL_UNPACK_ALIGNMENT, oldAlign ) );

	//---------------------------------------------------------------------
	// Generate our face/render data
	//---------------------------------------------------------------------

	glFaces.resize( map->data.numFaces );
	std::vector< bspVertex_t > vertexData( &map->data.vertexes[ 0 ], &map->data.vertexes[ map->data.numVertexes ] );

	// cache the data already used for any polygon or mesh faces, so we don't have to iterate through their index/vertex mapping every frame. For faces
	// which aren't of these two categories, we leave them be.
	for ( int32_t i = 0; i < map->data.numFaces; ++i )
	{
		const shaderInfo_t* shader = map->GetShaderInfo( i );
		mapModel_t* mod = &glFaces[ i ]; 

		const bspFace_t* face = map->data.faces + i;

		if ( face->type == BSP_FACE_TYPE_MESH || face->type == BSP_FACE_TYPE_POLYGON )
		{
			mod->indices.resize( face->numMeshVertexes, 0 );
			for ( int32_t j = 0; j < face->numMeshVertexes; ++j )
			{
				mod->indices[ j ] = face->vertexOffset + map->data.meshVertexes[ face->meshVertexOffset + j ].offset;
			}
		}
		else if ( face->type == BSP_FACE_TYPE_PATCH )
		{
			mod->vboOffset = ( GLuint ) vertexData.size();
			int width = ( face->size[ 0 ] - 1 ) / 2;
			int height = ( face->size[ 1 ] - 1 ) / 2;

            //GLenum bufferUsage = ( shader && shader->deform )? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;

			// ( k, j ) maps to a ( row, col ) index scheme referring to the beginning of a patch 
			int n, m;
			mod->controlPoints.resize( width * height * 9 );

			for ( n = 0; n < width; ++n )
			{
				for ( m = 0; m < height; ++m )
				{
					int baseSource = face->vertexOffset + 2 * m * width + 2 * n;
					int baseDest = ( m * width + n ) * 9;

					for ( int32_t c = 0; c < 3; ++c )
					{
						mod->controlPoints[ baseDest + c * 3 + 0 ] = &map->data.vertexes[ baseSource + c * face->size[ 0 ] + 0 ];
						mod->controlPoints[ baseDest + c * 3 + 1 ] = &map->data.vertexes[ baseSource + c * face->size[ 0 ] + 1 ];
						mod->controlPoints[ baseDest + c * 3 + 2 ] = &map->data.vertexes[ baseSource + c * face->size[ 0 ] + 2 ];
					}
					
					GenPatch( mod, shader, baseDest, ( int32_t ) vertexData.size() );
				}
			}

			const uint32_t L1 = mod->subdivLevel + 1;
			mod->rowIndices.resize( width * height * mod->subdivLevel, 0 );
			mod->trisPerRow.resize( width * height * mod->subdivLevel, 0 );

			for ( size_t row = 0; row < glFaces[ i ].rowIndices.size(); ++row )
			{
				mod->trisPerRow[ row ] = 2 * L1;
				mod->rowIndices[ row ] = &mod->indices[ row * 2 * L1 ];  
			}

            vertexData.insert( vertexData.end(), mod->patchVertices.begin(), mod->patchVertices.end() );
		}

		mod->CalcBounds( face->type, map->data );
	}

	//---------------------------------------------------------------------
	// Generate the index and draw indirect buffers
	//---------------------------------------------------------------------

	// Allocate vertex data from map and store it all in a single vbo
	GL_CHECK( glBindVertexArray( vao ) );
	GL_CHECK( glBindBuffer( GL_ARRAY_BUFFER, vbo ) );
    GL_CHECK( glBufferData( GL_ARRAY_BUFFER, sizeof( bspVertex_t ) * vertexData.size(), &vertexData[ 0 ], GL_DYNAMIC_DRAW ) );

	// NOTE: this vertex layout may not persist when the model program is used; so be wary of that. "main"
	// and "model" should both have the same attribute location values though

	glPrograms[ "main" ]->LoadAttribLayout();
}

void BSPRenderer::Render( void )
{ 
	double startTime = glfwGetTime();

    RenderPass( camera->ViewData(), false );
	
	frameTime = glfwGetTime() - startTime;

	frameCount++;
}

void BSPRenderer::RenderPass( const viewParams_t& view, bool envmap )
{
	GL_CHECK( glDisable( GL_CULL_FACE ) );
	GL_CHECK( glClearDepth( 1.0f ) );
    GL_CHECK( glClearColor( 0.0f, 0.0f, 0.0f, 0.0f ) );
	GL_CHECK( glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) );

    static auto LDrawList = [ this ]( drawSurfaceList_t& list ) -> void
	{
        DrawSurfaceList( list.surfaces );
        DrawSurfaceList( list.effectSurfaces );

		list.surfaces.clear();
		list.effectSurfaces.clear();
	};

    static auto LDrawClear = [ this ]( drawPass_t& pass ) -> void
	{
        LDrawList( pass.polymeshes );
        LDrawList( pass.patches );
		pass.shader = nullptr;
		pass.face = nullptr;
	};

	static auto LTraverseDraw = [ this ]( drawPass_t& pass, bool solid ) -> void
	{
		pass.isSolid = solid;
		DrawNode( pass, 0 );
		LDrawClear( pass );
	};

    drawPass_t pass( map, view );
	pass.envmap = envmap;
	pass.leaf = map->FindClosestLeaf( pass.view.origin );
	
	// Draw Models and Leaf Faces
	LoadTransforms( pass.view.transform, pass.view.clipTransform );

	frustum->Update( pass.view, true );

    pass.facesVisited.assign( pass.facesVisited.size(), 0 );

	for ( int32_t i = 1; i < map->data.numModels; ++i )
	{
		bspModel_t* model = &map->data.models[ i ];
		
		AABB bounds( model->boxMax, model->boxMin );

		if ( !frustum->IntersectsBox( bounds ) )
		{
			continue;
		}

		for ( int32_t j = 0; j < model->numFaces; ++j )
		{
			if ( pass.facesVisited[ model->faceOffset + j ] )
			{
				continue;
			}

			LoadPassParams( pass, model->faceOffset + j, PASS_DRAW_MAIN );
		
			if ( config.drawFacesOnly )
			{
				DrawFace( pass );
			}
			else 
			{
				drawSurfaceList_t& list = ( pass.face->type == BSP_FACE_TYPE_PATCH )? pass.patches: pass.polymeshes;

				AddSurface( pass.shader, pass.faceIndex, pass.shader? list.effectSurfaces: list.surfaces );
				pass.facesVisited[ pass.faceIndex ] = true;
			}
		}
	}

    pass.facesVisited.assign( pass.facesVisited.size(), 0 );

    LDrawClear( pass );

    pass.type = PASS_DRAW;

    LTraverseDraw( pass, true );
    LTraverseDraw( pass, false );
}

void BSPRenderer::Update( float dt )
{
	camera->Update();

    viewParams_t& view = camera->ViewDataMut();
	SetNearFar( view.clipTransform, 1.0f, 100000.0f );

    frustum->Update( view, false );

    deltaTime = dt;
}

void BSPRenderer::DrawNode( drawPass_t& pass, int32_t nodeIndex )
{
    if ( nodeIndex < 0 )
    {
		pass.viewLeafIndex = -( nodeIndex + 1 );
        const bspLeaf_t* viewLeaf = &map->data.leaves[ pass.viewLeafIndex ];

        if ( !map->IsClusterVisible( pass.leaf->clusterIndex, viewLeaf->clusterIndex ) )
		{
            return;
		}

		AABB leafBounds;
		leafBounds.maxPoint = glm::vec3( viewLeaf->boxMax.x, viewLeaf->boxMax.y, viewLeaf->boxMax.z );
		leafBounds.minPoint = glm::vec3( viewLeaf->boxMin.x, viewLeaf->boxMin.y, viewLeaf->boxMin.z );

        if ( !frustum->IntersectsBox( leafBounds ) )
		{
			return;
		}

        for ( int32_t i = 0; i < viewLeaf->numLeafFaces; ++i )
        {
            int32_t faceIndex = map->data.leafFaces[ viewLeaf->leafFaceOffset + i ].index;
			
			// if pass.facesVisited[ faceIndex ] is still false after this criteria's
			// evaluations, we'll pick it up on the next pass as it will meet
			// the necessary criteria then.
            if ( pass.facesVisited[ faceIndex ] )
                continue;
			
			LoadPassParams( pass, faceIndex, PASS_DRAW_MAIN );
			
            bool transparent = IsTransFace( pass.faceIndex, pass.shader );

            bool add = ( !pass.isSolid && transparent ) || ( pass.isSolid && !transparent );

            if ( add )
			{
				// Only draw individual faces if they're patches, since meshes and polygons
				// can be easily grouped together from the original vbo
				if ( ( pass.face->type == BSP_FACE_TYPE_PATCH && config.drawFacePatches ) || config.drawFacesOnly )
				{
					DrawFace( pass );
				}
				else
				{
					drawSurfaceList_t& list = ( pass.face->type == BSP_FACE_TYPE_PATCH )? pass.patches: pass.polymeshes;
					AddSurface( pass.shader, pass.faceIndex, pass.shader? list.effectSurfaces: list.surfaces );
				}
				pass.facesVisited[ pass.faceIndex ] = true;
			}
			else
			{
				pass.shader = nullptr;
				pass.face = nullptr;
				pass.faceIndex = 0;
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
            DrawNode( pass, node->children[ 0 ] );
			DrawNode( pass, node->children[ 1 ] );
		}
        else
        {
            DrawNode( pass, node->children[ 1 ] );
            DrawNode( pass, node->children[ 0 ] );
        }
    }
}

static std::array< glm::vec3, 6 > faceNormals = 
{
	glm::vec3( 1.0f, 0.0f, 0.0f ),
	glm::vec3( -1.0f, 0.0f, 0.0f ),
	glm::vec3( 0.0f, 1.0f, 0.0f ),
	glm::vec3( 0.0f, -1.0f, 0.0f ),
	glm::vec3( 0.0f, 0.0f, 1.0f ),
	glm::vec3( 0.0f, 0.0f, -1.0f ),
};

void BSPRenderer::DrawMapPass( int32_t textureIndex, int32_t lightmapIndex, std::function< void( const Program& mainRef ) > callback )
{
    const Program& main = *( glPrograms.at( "main" ) );

    glTextureArray->Bind( 0, "fragSampler", main );
    glLightmapArray->Bind( 1, "fragLightmapSampler", main );
    GL_CHECK( glBindSampler( 1, glLightmapArray->samplers[ 0 ] ) );

    GL_CHECK( glBlendFunc( GL_ONE, GL_ZERO ) );
    GL_CHECK( glDepthFunc( GL_LEQUAL ) );

    std::array< glm::vec3, 2 > fragBiases;

    if ( textureIndex >= 0 )
    {
        fragBiases[ 0 ] = glTextureArray->biases[ textureIndex ];
        GL_CHECK( glBindSampler( 0, glTextureArray->samplers[ textureIndex ] ) );
    }
    else
    {
        fragBiases[ 0 ] = glm::vec3( 0.0f, 0.0f, -1.0f );
        GL_CHECK( glBindSampler( 0, glTextureArray->samplers[ 0 ] ) );
    }

    if ( lightmapIndex >= 0 )
    {
        fragBiases[ 1 ] = glLightmapArray->biases[ lightmapIndex ];
    }
    else
    {
        fragBiases[ 1 ] = glm::vec3( 0.0f, 0.0f, -1.0f );
    }

    main.LoadVec3Array( "fragBiases", &fragBiases[ 0 ][ 0 ], 2 );

    main.Bind();

    callback( main );

    main.Release();

    glTextureArray->Release( 0 );
    GL_CHECK( glBindSampler( 0, 0 ) );

    glLightmapArray->Release( 1 );
    GL_CHECK( glBindSampler( 1, 0 ) );
}

void BSPRenderer::AddSurface( const shaderInfo_t* shader, int32_t faceIndex, std::vector< drawSurface_t >& surfList )
{	
	const bspFace_t* face = &map->data.faces[ faceIndex ]; 

	bool add = true;

	for ( drawSurface_t& surf: surfList )
	{
		if ( shader == surf.shader && face->texture == surf.textureIndex 
			&& face->lightmapIndex == surf.lightmapIndex && face->type == surf.faceType )
		{
			AddSurfaceData( surf, faceIndex, glFaces );
			add = false;
			break;
		}
	}

	if ( add )
	{
		drawSurface_t surf;

		surf.shader = shader;
		surf.lightmapIndex = face->lightmapIndex;
		surf.textureIndex = face->texture;
		surf.faceType = face->type;

		AddSurfaceData( surf, faceIndex, glFaces );
		surfList.push_back( std::move( surf ) ); 
	}
}

void BSPRenderer::DrawSurface( const drawSurface_t& surf, const shaderStage_t* stage, const Program& program ) const
{
    for ( int32_t i: surf.faceIndices )
    {
        DeformVertexes( glFaces[ i ], surf.shader );
    }

    if ( stage && stage->tcgen == TCGEN_ENVIRONMENT )
    {
        glm::vec3 n( map->data.faces[ surf.faceIndices[ 0 ] ].normal );
        program.LoadVec3( "surfaceNormal", n );
    }

    program.LoadAttribLayout();

    GLenum mode = ( surf.faceType == BSP_FACE_TYPE_PATCH )? GL_TRIANGLE_STRIP: GL_TRIANGLES;

    GL_CHECK( glMultiDrawElements( mode, &surf.indexBufferSizes[ 0 ],
        GL_UNSIGNED_INT, ( const GLvoid** ) &surf.indexBuffers[ 0 ], surf.indexBuffers.size() ) );
}


static INLINE void EffectPassTexFromArray( glm::vec2& dims, 
	int32_t index, const textureArray_t& texArray, const Program& program )
{
	if ( index < 0 )
	{
		index = ( int32_t ) texArray.biases.size() - 1;
	}

	const glm::vec3& bias = texArray.biases[ index ]; 
	texArray.Bind( 0, "sampler0", program ); 
	program.LoadVec3( "bias", bias );
	GL_CHECK( glBindSampler( 0, texArray.samplers[ index ] ) );
	dims.x = bias.x;
	dims.y = bias.y;
}

void BSPRenderer::DrawEffectPass( const drawTuple_t& data, BSPRenderer::drawCall_t callback )
{
    const shaderInfo_t* shader = std::get< 1 >( data );
    int lightmapIndex = std::get< 3 >( data );

	// Each effect pass is allowed only one texture, so we don't need a second texcoord
	GL_CHECK( glDisableVertexAttribArray( 3 ) );

    if  ( shader->cullFace )
    {
        GL_CHECK( glEnable( GL_CULL_FACE ) );
        GL_CHECK( glCullFace( shader->cullFace ) );
        GL_CHECK( glFrontFace( GL_CCW ) );
    }

    for ( int32_t i = 0; i < shader->stageCount; ++i )
	{	
		const shaderStage_t& stage = shader->stageBuffer[ i ];
		const Program& stageProg = *( stage.program.get() );

        GL_CHECK( glBlendFunc( stage.blendSrc, stage.blendDest ) );
		GL_CHECK( glDepthFunc( stage.depthFunc ) );	

		glm::vec2 texDims( 0 );
		
		bool usingAtlas = stage.mapType == MAP_TYPE_IMAGE;

		if ( usingAtlas )
		{
			//EffectPassTexFromArray( texDims, 
			//	stage.textureIndex, *( glShaderArray.get() ), stageProg );
		
			const gTextureImage_t& texParams = GTextureImage( theTexture, stage.textureIndex );
			glm::vec2 invRowPitch( GTextureInverseRowPitch( theTexture ) );

			glm::vec4 transform;
			transform.x = texParams.stOffsetStart.s;
			transform.y = texParams.stOffsetStart.t;
			transform.z = invRowPitch.x;
			transform.w = invRowPitch.y;

			GBindTexture( theTexture );
			stageProg.LoadInt( "sampler0", 0 );
			stageProg.LoadVec4( "imageTransform", transform );
			stageProg.LoadVec2( "imageScaleRatio", texParams.imageScaleRatio );
		}
		else
		{
			EffectPassTexFromArray( texDims, 
				lightmapIndex, *( glLightmapArray.get() ), stageProg );
		}

		for ( effect_t e: stage.effects )
		{
			if ( e.name == "tcModScroll" )
			{
				e.data.xyzw[ 2 ] = texDims.x;
				e.data.xyzw[ 3 ] = texDims.y;
			}
			else if ( e.name == "tcModRotate" )
			{
				e.data.rotation2D.center[ 0 ] = 0.5f;
				e.data.rotation2D.center[ 1 ] = 0.5f;
			}

            glEffects.at( e.name )( stageProg, e );
		}	

		stageProg.Bind();
        callback( std::get< 0 >( data ), stageProg, &stage );
		stageProg.Release();

		if ( usingAtlas )
			GReleaseTexture( theTexture );
	}

	GL_CHECK( glEnableVertexAttribArray( 3 ) );

	GL_CHECK( glActiveTexture( GL_TEXTURE0 ) );
	GL_CHECK( glBindTexture( GL_TEXTURE_2D, 0 ) );
	GL_CHECK( glBindSampler( 0, 0 ) );

	GL_CHECK( glActiveTexture( GL_TEXTURE0 + 1 ) );
	GL_CHECK( glBindTexture( GL_TEXTURE_2D, 0 ) );
	GL_CHECK( glBindSampler( 1, 0 ) );

    GL_CHECK( glBlendFunc( GL_ONE, GL_ZERO ) );

	GL_CHECK( glDisable( GL_CULL_FACE ) );
}

void BSPRenderer::DrawFace( drawPass_t& pass )
{
	GL_CHECK( glBlendFunc( GL_ONE, GL_ZERO ) );
	GL_CHECK( glDepthFunc( GL_LEQUAL ) );

	switch ( pass.drawType )
	{
		case PASS_DRAW_EFFECT:
        {
            drawTuple_t data = std::make_tuple( nullptr, pass.shader, pass.face->texture, pass.face->lightmapIndex );

            DrawEffectPass( data, [ &pass, this ]( const void* param, const Program& prog, const shaderStage_t* stage )
            {
                UNUSED( param );
                DrawFaceVerts( pass, stage, prog );

                /*
                if ( config.drawFaceBounds )
                {
                    DrawFaceBounds( pass.view, pass.faceIndex );
                }
                */
            });
        }
            break;
	
		case PASS_DRAW_MAIN:

            DrawMapPass( pass.face->texture, pass.face->lightmapIndex, [ &pass, this ]( const Program& main )
            {
                DrawFaceVerts( pass, nullptr, main );
            });

			break;
	}

    pass.facesVisited[ pass.faceIndex ] = 1;
}

void BSPRenderer::DrawSurfaceList( const std::vector< drawSurface_t >& list )
{	
    auto LEffectCallback = [ this ]( const void* voidsurf, const Program& prog, const shaderStage_t* stage )
    {
        const drawSurface_t& surf = *( ( const drawSurface_t* )( voidsurf ) );

        DrawSurface( surf, stage, prog );

        /*
        if ( config.drawFaceBounds )
        {
            for ( int32_t i: surf.faceIndices )
            {
                DrawFaceBounds( pass.view, i );
            }
        }
        */
    };

	for ( const drawSurface_t& surf: list )
	{
		if ( surf.shader )
		{
            drawTuple_t tuple = std::make_tuple( ( const void* )&surf, surf.shader, surf.textureIndex, surf.lightmapIndex );
			DrawEffectPass( tuple, LEffectCallback );	
		}
		else
		{
            DrawMapPass( surf.textureIndex, surf.lightmapIndex, [ &surf, this ]( const Program& main )
            {
                DrawSurface( surf, nullptr, main );
            });  
		}
	}

	glTextureArray->Release( 0 );
}

void BSPRenderer::DrawFaceVerts( const drawPass_t& pass, const shaderStage_t* stage, const Program& program ) const
{
    if ( stage && stage->tcgen == TCGEN_ENVIRONMENT )
    {
        program.LoadVec3( "surfaceNormal", pass.face->normal );
    }

	const mapModel_t& m = glFaces[ pass.faceIndex ];

	if ( pass.face->type == BSP_FACE_TYPE_POLYGON || pass.face->type == BSP_FACE_TYPE_MESH )
	{
		GL_CHECK( glDrawElements( GL_TRIANGLES, m.indices.size(), GL_UNSIGNED_INT, &m.indices[ 0 ] ) );
	}
	else if ( pass.face->type == BSP_FACE_TYPE_PATCH )
	{
        if ( pass.shader && pass.shader->deform )
		{
			DeformVertexes( m, pass.shader );
		}
		
		program.LoadAttribLayout();

		GL_CHECK( glMultiDrawElements( GL_TRIANGLE_STRIP, 
			&m.trisPerRow[ 0 ], GL_UNSIGNED_INT, ( const GLvoid** ) &m.rowIndices[ 0 ], m.trisPerRow.size() ) );
	}
}

void BSPRenderer::DrawFaceBounds( const viewParams_t& view, int32_t faceIndex ) const
{
	ImPrep( view.transform, view.clipTransform );

	glm::vec4 color;
	
	float t = glFaces[ faceIndex ].bounds.CalcIntersection( glm::normalize( view.forward ), view.origin );
	
	if ( t != FLT_MAX )
	{
		color = glm::vec4( 0.0f, 1.0f, 0.0f, 1.0f );
	}
	else
	{
		color = glm::vec4( 0.0f, 0.0f, 1.0f, 1.0f );
	}

	ImDrawAxes( 100.0f );
	
	ImDrawBounds( glFaces[ faceIndex ].bounds, color );
}

void BSPRenderer::DeformVertexes( const mapModel_t& m, const shaderInfo_t* shader ) const
{
    if ( !shader || shader->deformCmd == VERTEXDEFORM_CMD_UNDEFINED )
        return;

    std::vector< bspVertex_t > verts = m.patchVertices;

	for ( uint32_t i = 0; i < verts.size(); ++i )
	{
		glm::vec3 n( verts[ i ].normal * GenDeformScale( verts[ i ].position, shader ) );

        verts[ i ].position += n;
	}

	UpdateBufferObject< bspVertex_t >( GL_ARRAY_BUFFER, vbo, m.vboOffset, verts, false );
}

void BSPRenderer::LoadLightVol( const drawPass_t& pass, const Program& prog ) const
{
	if ( pass.lightvol )
	{
		float phi = glm::radians( ( float )pass.lightvol->direction.x * 4.0f ); 
		float theta = glm::radians( ( float )pass.lightvol->direction.y * 4.0f );
		
		glm::vec3 dirToLight( glm::cos( theta ) * glm::cos( phi ), glm::sin( phi ), glm::cos( phi ) * glm::sin( theta ) );
		
		glm::vec3 ambient( pass.lightvol->ambient );
		ambient *= Inv255< float >();

		glm::vec3 directional( pass.lightvol->directional );
		directional *= Inv255< float >();

		prog.LoadVec3( "fragDirToLight", dirToLight );
		prog.LoadVec3( "fragAmbient", ambient );
		prog.LoadVec3( "fragDirectional", directional );
	}
}

/*
int BSPRenderer::CalcLightvolIndex( const drawPass_t& pass ) const
{
	const glm::vec3& max = map->data.models[ 0 ].boxMax;
	const glm::vec3& min = map->data.models[ 0 ].boxMin;
		 
	glm::vec3 input;
	input.x = glm::abs( glm::floor( pass.view.origin.x * Inv64< float >() ) - glm::ceil( min.x * Inv64< float >() ) ); 
	input.y = glm::abs( glm::floor( pass.view.origin.y * Inv64< float >() ) - glm::ceil( min.y * Inv64< float >() ) ); 
	input.z = glm::abs( glm::floor( pass.view.origin.z * Inv128< float >() ) - glm::ceil( min.z * Inv128< float >() ) ); 
		
	glm::vec3 interp = input / lightvolGrid;

	glm::ivec3 dindex;
	dindex.x = static_cast< int32_t >( interp.x * lightvolGrid.x );
	dindex.y = static_cast< int32_t >( interp.y * lightvolGrid.y );
	dindex.z = static_cast< int32_t >( interp.z * lightvolGrid.z );
	
	// Performs an implicit cast from a vec3 to ivec3
	glm::ivec3 dims( lightvolGrid );

	return ( dindex.z * dims.x * dims.y + dims.x * dindex.y + dindex.x ) % map->data.numLightvols;
}
*/
