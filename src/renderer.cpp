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
	true
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
		for ( const bspVertex_t& v: vertices )
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
lightSampler_t::lightSampler_t( void )
	:	targetPlane( 0.0f, 0.0f, 0.0f, 1.0f ),
		boundsMin( 0.0f ), boundsMax( 0.0f ),
		fbos( { 0, 0 } )
{
	GLint viewport[ 4 ];
	GL_CHECK( glGetIntegerv( GL_VIEWPORT, viewport ) );

	GLint cubeDims = glm::max( viewport[ 2 ], viewport[ 3 ] );

	attachments[ 0 ].mipmap = false;
	attachments[ 0 ].SetBufferSize( viewport[ 2 ], viewport[ 3 ], 4, 0 );
	attachments[ 0 ].Load2D();

	attachments[ 1 ].mipmap = false;
	attachments[ 1 ].SetBufferSize( cubeDims, cubeDims, 4, 255 );
	attachments[ 1 ].LoadCubeMap();

	GL_CHECK( glGenFramebuffers( lightSampler_t::NUM_BUFFERS, &fbos[ 0 ] ) );
	
	GL_CHECK( glBindFramebuffer( GL_FRAMEBUFFER, fbos[ 0 ] ) );
	GL_CHECK( glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
		GL_TEXTURE_2D, attachments[ 0 ].handle, 0 ) );
	GL_CHECK( glBindFramebuffer( GL_FRAMEBUFFER, 0 ) );
}

lightSampler_t::~lightSampler_t( void )
{
	GL_CHECK( glDeleteFramebuffers( lightSampler_t::NUM_BUFFERS, &fbos[ 0 ] ) );
}

void lightSampler_t::Bind( int32_t fbo ) const
{
	GL_CHECK( glBindFramebuffer( GL_FRAMEBUFFER, fbos[ fbo ] ) );
	GL_CHECK( glDrawBuffer( GL_COLOR_ATTACHMENT0 ) );
}

void lightSampler_t::Release( void ) const
{
	GL_CHECK( glBindFramebuffer( GL_FRAMEBUFFER, 0 ) );
	GL_CHECK( glDrawBuffer( GL_BACK ) );
}

// Create a view projection transform which looks up at the sky
// and fills the screen with as much space as possible so the FBO
// can be sampled from
void lightSampler_t::Elevate( const glm::vec3& min, const glm::vec3& max )
{
	glm::vec3 a( glm::vec3( min.x, max.y, max.z ) - max );
	glm::vec3 b( glm::vec3( max.x, max.y, min.z ) - max );
	glm::vec3 n( -glm::cross( a, b ) );

	targetPlane = glm::vec4( n, glm::dot( n, max ) );
	boundsMin = glm::vec2( min.x, min.z );
	boundsMax = glm::vec2( max.x, max.z );

	float w, h;
	float xDist = max.x - min.x;
	float zDist = min.z - max.z;
	float yDist = max.y - min.y;

	glm::vec3 up;

	if ( xDist > zDist )
	{
		w = xDist;
		h = zDist;
		up = glm::vec3( min.x, 0.0f, max.z ) - glm::vec3( min.x, 0.0f, min.z );
	}
	else
	{
		w = zDist;
		h = xDist;
		up = glm::vec3( max.x, 0.0f, max.z ) - glm::vec3( min.x, 0.0f, max.z );
	}
	
	up = glm::normalize( up );

	camera.SetClipTransform( glm::ortho< float >( -w * 0.5f, w * 0.5f, -h * 0.5f, h * 0.5f, 0.0f, 1000000.0f ) );

	glm::vec3 eye( ( min + max ) * 0.5f );
	eye.y += ( max.y - min.y ) * 0.3f;

	glm::vec3 target( eye + glm::vec3( 0.0f, yDist, 0.0f ) );

	camera.SetViewTransform( glm::lookAt( eye, target, up ) );
	camera.SetViewOrigin( eye ); 
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
	std::vector< texture_t >& textures, int32_t width, int32_t height )
{
	textures.push_back( glDummyTexture );

	texArray.reset( new textureArray_t( width, height, textures.size(), false ) ); 
	for ( uint32_t i = 0; i < textures.size(); ++i )
	{
		const texture_t& tex = textures[ i ];
		if ( !tex.pixels.empty() )
		{
			GLuint sampler = tex.sampler;
			if ( !sampler )
			{
				sampler = GenSampler( tex.mipmap, tex.wrap );
			}

			glm::ivec3 dims( tex.width, tex.height, i );
			texArray->LoadSlice( sampler, dims, tex.pixels, false );
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

	std::vector< texture_t > shaderTextures;
	glm::ivec2 shaderMegaDims = Shader_LoadAll( &map->data, shaderTextures, map->effectShaders, mapLoadFlags );
	
	glDummyTexture.SetBufferSize( 64, 64, 4, 255 );
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

				if ( glTextures[ t ].LoadFromFile( str.c_str(), mapLoadFlags ) )
				{
					width = glm::max( width, glTextures[ t ].width );
					height = glm::max( height, glTextures[ t ].height );
					success = true;
					
					glTextures[ t ].wrap = GL_REPEAT;
					glTextures[ t ].minFilter = GL_LINEAR_MIPMAP_LINEAR;
					
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
		glLightmaps[ l ].SetBufferSize( BSP_LIGHTMAP_WIDTH, BSP_LIGHTMAP_HEIGHT, 4, 255 );
		
		Pixels_24BitTo32Bit( &glLightmaps[ l ].pixels[ 0 ], 
			&map->data.lightmaps[ l ].map[ 0 ][ 0 ][ 0 ], BSP_LIGHTMAP_WIDTH * BSP_LIGHTMAP_HEIGHT );

		glLightmaps[ l ].wrap = GL_REPEAT;
		glLightmaps[ l ].minFilter = GL_LINEAR_MIPMAP_LINEAR;
	}
	LoadTextureArray( glLightmapArray, glLightmaps, BSP_LIGHTMAP_WIDTH, BSP_LIGHTMAP_HEIGHT );

	GL_CHECK( glPixelStorei( GL_UNPACK_ALIGNMENT, oldAlign ) );

	//---------------------------------------------------------------------
	// Generate our face/render data
	//---------------------------------------------------------------------

	glFaces.resize( map->data.numFaces );
	std::vector< bspVertex_t > vertexData( &map->data.vertexes[ 0 ], &map->data.vertexes[ map->data.numVertexes ] );

	int indexOffset = ( int32_t ) vertexData.size();

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

			GLenum bufferUsage = ( shader && shader->deform )? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;

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

			vertexData.insert( vertexData.end(), mod->vertices.begin(), mod->vertices.end() );
		}

		mod->CalcBounds( face->type, map->data );

		if ( shader && !!( shader->surfaceParms & SURFPARM_ENVMAP ) )
		{
			glm::vec3 center( mod->bounds.maxPoint );
			glm::vec3 target( mod->bounds.maxPoint + face->normal );

			glm::mat4 view( glm::lookAt( center, target, CalcUpVector( face->normal, glm::vec3( 0.0f, 1.0f, 0.0f ) ) ) ); 

			mod->envmap.reset( new rtt_t( GL_COLOR_ATTACHMENT0, view ) );
			mod->envmap->Attach( 1920, 1024, 4 );
		}
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

	const bspNode_t* root = &map->data.nodes[ 0 ];

	glm::vec3 min( map->data.nodes[ 0 ].boxMin );
	glm::vec3 max( map->data.nodes[ 0 ].boxMax );

	lightSampler.Elevate( min, max );
}

void BSPRenderer::Sample( void )
{
	// Clear the color buffer to black so that any sample hits outside of the sky 
	// won't contribute anything in the irradiance generation shader. Depth range
	// is changed so that only the sky is seen

	curView = VIEW_LIGHT_SAMPLE;
	lightSampler.Bind( 0 );

	Render();
	lightSampler.Release();

	curView = VIEW_MAIN;
}

void BSPRenderer::Render( void )
{ 
	double startTime = glfwGetTime();

	/*
	GLint width = GLint( CameraFromView()->ViewData().width * 0.5f );
	GLint height = GLint( CameraFromView()->ViewData().height );
	viewportStash_t viewStash( 0, 0, width, height ); 
	*/

	RenderPass( CameraFromView()->ViewData(), false );
	
	frameTime = glfwGetTime() - startTime;

	frameCount++;
}

void BSPRenderer::RenderPass( const viewParams_t& view, bool envmap )
{
	GL_CHECK( glDisable( GL_CULL_FACE ) );
	GL_CHECK( glClearDepth( 1.0f ) );
	GL_CHECK( glClearColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
	GL_CHECK( glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) );

	static auto LDrawList = [ this ]( drawPass_t& pass, drawSurfaceList_t& list ) -> void
	{
		DrawSurfaceList( pass, list.surfaces );
		DrawSurfaceList( pass, list.effectSurfaces );

		list.surfaces.clear();
		list.effectSurfaces.clear();
	};

	static auto LDrawClear = [ this ]( drawPass_t& pass ) -> void
	{
		LDrawList( pass, pass.polymeshes );
		LDrawList( pass, pass.patches );
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

	LDrawClear( pass );
	
	pass.type = PASS_DRAW;

	LTraverseDraw( pass, true );
	LTraverseDraw( pass, false );
}

void BSPRenderer::Update( float dt )
{
	camera->Update();

	viewParams_t& view = CameraFromView()->ViewDataMut();
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
			
			bool add = ( !pass.isSolid && IsTransFace( pass.faceIndex, pass.shader ) ) 
				    || ( pass.isSolid && !IsTransFace( pass.faceIndex, pass.shader ) );

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

void BSPRenderer::DrawMapPass( drawPass_t& pass )
{
	int best = 0;
	if ( config.drawIrradiance )
	{
		float cosAng = 0.0f;
		const glm::vec3 drawFaceNormal( pass.face->normal );
		for ( int32_t i = 0; i < 6; ++i )
		{
			float c = glm::clamp( glm::dot( faceNormals[ i ], drawFaceNormal ), 0.0f, 1.0f );
			if ( c > cosAng )
			{
				best = i;
				cosAng = c;
			}
		}

		glPrograms[ "irradiate" ]->LoadAttribLayout();

		lightSampler.Bind( 1 );

		GL_CHECK( glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + best, lightSampler.attachments[ 1 ].handle, 0 ) );

		GL_CHECK( glActiveTexture( GL_TEXTURE0 ) );
		lightSampler.attachments[ 0 ].Bind();
		GL_CHECK( glBindSampler( 0, lightSampler.attachments[ 0 ].sampler ) );

		const Program& irradiate = *( glPrograms[ "irradiate" ].get() );

		irradiate.LoadInt( "fragRadianceSampler", 0 );
		irradiate.LoadVec2( "fragMin", lightSampler.boundsMin );
		irradiate.LoadVec2( "fragMax", lightSampler.boundsMax );
		irradiate.LoadVec4( "fragTargetPlane", lightSampler.targetPlane );
		irradiate.Bind();

		DrawFaceVerts( pass, irradiate );

		irradiate.Release();

		lightSampler.Release();

		lightSampler.attachments[ 0 ].Release();
		GL_CHECK( glBindSampler( 0, 0 ) );
	}
	
	const texture_t* tex0 = nullptr;
	const texture_t* tex1 = nullptr;

	BeginMapPass( pass, &tex0, &tex1 );
	DrawFaceVerts( pass, *( glPrograms[ "main" ].get() ) );
	EndMapPass( pass, tex0, tex1 );
}

void BSPRenderer::BeginMapPass( drawPass_t& pass, const texture_t** tex0, const texture_t** tex1 )
{
	if ( glTextures[ pass.face->texture ].handle )
	{
	 	*tex0 = &glTextures[ pass.face->texture ]; 
	}
	else
	{
		*tex0 = &glDummyTexture;
	}

	if ( pass.face->lightmapIndex >= 0 )
	{
		*tex1 = &glLightmaps[ pass.face->lightmapIndex ];
	}
	else
	{
		*tex1 = &glDummyTexture;
	}

	const Program& main = *( glPrograms[ "main" ].get() );

	( *tex0 )->Bind( 0, "fragTexSampler", main );
	( *tex1 )->Bind( 1, "fragLightmapSampler", main ); 

	main.LoadAttribLayout();
	main.Bind();
}

void BSPRenderer::EndMapPass( drawPass_t& pass, const texture_t* tex0, const texture_t* tex1 )
{
	glPrograms[ "main" ]->Release();
	
	tex0->Release( 0 );
	tex1->Release( 1 );
	
	//lightSampler.attachments[ 1 ].Release( 1 );
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

void BSPRenderer::ReflectFromTuple( const drawTuple_t& data, const drawPass_t& pass, const Program& program )
{
	if ( pass.envmap )
	{
		glDummyTexture.Bind( 1, "samplerReflect", program );
		return;
	}

	/*
	GLint width = GLint( pass.view.width * 0.5f );
	GLint height = GLint( pass.view.height );
	viewportStash_t viewStash( width, 0, width, height ); 
	*/

	switch ( std::get< 0 >( data ) )
	{
	case OBJECT_SURFACE:
        {
            const drawSurface_t& surf = *( ( const drawSurface_t* ) std::get< 1 >( data ) );

            for ( int32_t face: surf.faceIndices )
            {
                mapModel_t& model = glFaces[ face ];
                if ( model.envmap
                    && model.bounds.CalcIntersection( glm::normalize( pass.view.forward ), pass.view.origin ) )
                {
                    //glm::mat4 view( glm::lookAt( model.bounds.maxPoint,
                        //model.bounds.maxPoint + pass.view.origin, CalcUpVector( pass.view.origin, glm::vec3( 0.0f, 1.0f, 0.0f ) ) ) );

                    model.envmap->Bind();

                    {
                        InputCamera camera( pass.view.width,
                            pass.view.height, model.envmap->view, CameraFromView()->ViewData().clipTransform );
                        RenderPass( camera.ViewData(), true );
                    }
                    //glDummyTexture.Bind( 1, "samplerReflect", program );

                    model.envmap->Release();
                    model.envmap->texture.Bind( 1, "samplerReflect", program );

                    break;
                }
            }
        }

		break;
    default: // compiler
            break;
	}
}

void BSPRenderer::DrawFromTuple( const drawTuple_t& data, const drawPass_t& pass, const Program& program )
{
	switch ( std::get< 0 >( data ) )
	{
		case OBJECT_FACE:
			DrawFaceVerts( pass, program );
			if ( config.drawFaceBounds )
			{
				DrawFaceBounds( pass.view, pass.faceIndex );
			}
			break;

		case OBJECT_SURFACE:
		{
			const drawSurface_t& surf = *( ( const drawSurface_t* ) std::get< 1 >( data ) );
			DrawSurface( surf, program );
			for ( int32_t i: surf.faceIndices )
			{
				if ( config.drawFaceBounds )
				{
					DrawFaceBounds( pass.view, i );
				}
			}
		}
		break;
	}
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

void BSPRenderer::DrawEffectPass( const drawPass_t& pass, const drawTuple_t& data )
{
	const shaderInfo_t* shader = std::get< 2 >( data );
	int lightmapIndex = std::get< 4 >( data );

	// Each effect pass is allowed only one texture, so we don't need a second texcoord
	GL_CHECK( glDisableVertexAttribArray( 3 ) );

	for ( int32_t i = 0; i < shader->stageCount; ++i )
	{	
		const shaderStage_t& stage = shader->stageBuffer[ i ];
		const Program& stageProg = *( stage.program.get() );

		if ( stage.tcgen == TCGEN_ENVIRONMENT )
		{
			ReflectFromTuple( data, pass, stageProg );
		}

		if  ( shader->cullFace )
		{
			GL_CHECK( glEnable( GL_CULL_FACE ) );
			GL_CHECK( glCullFace( shader->cullFace ) );
			GL_CHECK( glFrontFace( GL_CCW ) );
		}

		GL_CHECK( glBlendFunc( stage.rgbSrc, stage.rgbDest ) );
		GL_CHECK( glDepthFunc( stage.depthFunc ) );	

		glm::vec2 texDims( 0 );
		
		if ( stage.mapType == MAP_TYPE_IMAGE )
		{
			EffectPassTexFromArray( texDims, 
				stage.textureIndex, *( glShaderArray.get() ), stageProg );
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

			glEffects.at( e.name )( *( stage.program.get() ), e ); 
		}	

		stageProg.Bind();
		DrawFromTuple( data, pass, stageProg );
		stageProg.Release();
	}
	
	/*
	if ( envmap )
	{
		loadBlend_t blend( GL_ONE, GL_ZERO );

		const Program& prog = *( glPrograms.at( "debug" ).get() );
		prog.LoadVec4( "fragColor", glm::vec4( 0.0f, 1.0f, 0.0f, 1.0f ) );

		prog.Bind();	
		DrawFromTuple( data, pass, prog );
		prog.Release();
	}
	*/

	GL_CHECK( glEnableVertexAttribArray( 3 ) );

	GL_CHECK( glActiveTexture( GL_TEXTURE0 ) );
	GL_CHECK( glBindTexture( GL_TEXTURE_2D, 0 ) );
	GL_CHECK( glBindSampler( 0, 0 ) );

	GL_CHECK( glActiveTexture( GL_TEXTURE0 + 1 ) );
	GL_CHECK( glBindTexture( GL_TEXTURE_2D, 0 ) );
	GL_CHECK( glBindSampler( 1, 0 ) );

	GL_CHECK( glDisable( GL_CULL_FACE ) );
}

void BSPRenderer::DrawFace( drawPass_t& pass )
{
	GL_CHECK( glBlendFunc( GL_ONE, GL_ZERO ) );
	GL_CHECK( glDepthFunc( GL_LEQUAL ) );

	drawTuple_t data = std::make_tuple( OBJECT_FACE, 
		( const void* ) pass.face, pass.shader, pass.face->texture, pass.face->lightmapIndex );

	switch ( pass.drawType )
	{
		case PASS_DRAW_EFFECT:
			DrawEffectPass( pass, data );
			break;
	
		case PASS_DRAW_MAIN:
			DrawMapPass( pass );
			break;
	}

    pass.facesVisited[ pass.faceIndex ] = 1;
}

void BSPRenderer::DrawSurfaceList( const drawPass_t& pass, const std::vector< drawSurface_t >& list )
{	
	const Program& main = *( glPrograms.at( "main" ).get() );

	for ( const drawSurface_t& surf: list )
	{
		if ( surf.shader )
		{
			drawTuple_t tuple = std::make_tuple( OBJECT_SURFACE, 
				( const void* )&surf, surf.shader, surf.textureIndex, surf.lightmapIndex );
			DrawEffectPass( pass, tuple );
		}
		else
		{
			glTextureArray->Bind( 0, "fragSampler", main );
			glLightmapArray->Bind( 1, "fragLightmapSampler", main );
			GL_CHECK( glBindSampler( 1, glLightmaps[ 0 ].sampler ) );

			GL_CHECK( glBlendFunc( GL_ONE, GL_ZERO ) );
			GL_CHECK( glDepthFunc( GL_LEQUAL ) );

			std::array< glm::vec3, 2 > fragBiases;

			if ( surf.textureIndex >= 0 )
			{
				fragBiases[ 0 ] = glTextureArray->biases[ surf.textureIndex ];
				GL_CHECK( glBindSampler( 0, glTextureArray->samplers[ surf.textureIndex ] ) ); 
			}
			else
			{
				fragBiases[ 0 ] = glm::vec3( 0.0f, 0.0f, -1.0f );
				GL_CHECK( glBindSampler( 0, glDummyTexture.sampler ) );
			} 

			if ( surf.lightmapIndex >= 0 )
			{
				fragBiases[ 1 ] = glLightmapArray->biases[ surf.lightmapIndex ];
			}
			else
			{
				fragBiases[ 1 ] = glm::vec3( 0.0f, 0.0f, -1.0f );
			} 

			main.LoadVec3Array( "fragBiases", &fragBiases[ 0 ][ 0 ], 2 ); 
				
			main.Bind();
			DrawSurface( surf, main );
			main.Release();

			glTextureArray->Release( 0 );
			GL_CHECK( glBindSampler( 0, 0 ) );

			glLightmapArray->Release( 1 );
			GL_CHECK( glBindSampler( 1, 0 ) );
		}
	}

	glTextureArray->Release( 0 );
}

void BSPRenderer::DrawFaceVerts( const drawPass_t& pass, const Program& program ) const
{
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
	
	/*
	switch ( map->data.faces[ faceIndex ].type )
	{
	case BSP_FACE_TYPE_POLYGON:
		color = glm::vec4( 1.0f, 0.0f, 0.0f, 1.0f );
		break;
	case BSP_FACE_TYPE_PATCH:
		color = glm::vec4( 0.0f, 1.0f, 0.0f, 1.0f );
		break;
	case BSP_FACE_TYPE_MESH:
		color = glm::vec4( 0.0f, 0.0f, 1.0f, 1.0f );
		break;
	default:
		color = glm::vec4( 1.0f );
		break;
	}*/



	ImDrawBounds( glFaces[ faceIndex ].bounds, color );
}

void BSPRenderer::DeformVertexes( const mapModel_t& m, const shaderInfo_t* shader ) const
{
	std::vector< bspVertex_t > verts = m.vertices;
	
	int32_t stride = m.subdivLevel + 1;
	int32_t numPatchVerts = stride * stride;
	int32_t numPatches = verts.size() / numPatchVerts;

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
