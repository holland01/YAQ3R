#include "renderer.h"
#include "shader.h"
#include "io.h"
#include "lib/math.h"
#include "effect_shader.h"
#include "deform.h"
#include "model.h"
#include "renderer/shader_gen.h"
#include "renderer/context_window.h"
#include "extern/gl_atlas.h"
#include <glm/gtx/string_cast.hpp>
#include <fstream>
#include <random>
#include <algorithm>

//--------------------------------------------------------------
// debugging
//--------------------------------------------------------------

template < bool doIt >
struct logEffectPass_t 
{
	std::stringstream errorInfo;

	logEffectPass_t( const shaderInfo_t* shader )
	{
		if ( doIt )
		{
			errorInfo << shader->GetInfoString();
		}
	}

	void Push( int i, const shaderStage_t& stage )
	{
		if ( doIt )
		{
			errorInfo << "\n-------------------\n\n[" << i << "] "
				<< stage.GetInfoString() << "\n";
		}
	}

	~logEffectPass_t( void )
	{
		if ( doIt )
		{
			MLOG_INFO_ONCE( "%s", errorInfo.str().c_str() );
		}
	}
};

struct config_t
{
	bool drawFacesOnly: 1;
	bool drawAtlasTextureBoxes: 1;
	bool logStageTexCoordData: 1;
	bool debugRender;
};

static config_t gConfig =
{
	true,
	false,
	false,
	false
};

struct counts_t
{
	uint32_t numSolidEffect;
	uint32_t numSolidNormal;
	uint32_t numTransEffect;
	uint32_t numTransNormal;
};

static counts_t gCounts = { 0, 0, 0, 0 };
static uint64_t frameCount = 0;

//--------------------------------------------------------------
// drawPass_t
//--------------------------------------------------------------
drawPass_t::drawPass_t( const Q3BspMap& map, const viewParams_t& viewData )
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
	facesVisited.resize( map.data.numFaces, 0 );
}

//--------------------------------------------------------------
// RenderBase
//--------------------------------------------------------------
RenderBase::RenderBase( Q3BspMap& m )
	:	apiHandles( {{ 0, 0 }} ),
		map( m ),
		frustum( new Frustum() )
{
}

void RenderBase::MakeProg( const std::string& name, const std::string& vertSrc,
	const std::string& fragSrc, const std::vector< std::string >& uniforms,
	const std::vector< std::string >& attribs )
{
	glPrograms[ name ] = std::unique_ptr< Program >( new Program( vertSrc,
		fragSrc, uniforms, attribs ) );
}

std::string RenderBase::GetBinLayoutString( void ) const
{
	std::stringstream ss;

	ss << "[RenderBase]\n";
	ss << SSTREAM_BYTE_OFFSET( RenderBase, glPrograms );
	ss << SSTREAM_BYTE_OFFSET( RenderBase, apiHandles );
	ss << SSTREAM_BYTE_OFFSET( RenderBase, map );
	ss << SSTREAM_BYTE_OFFSET( RenderBase, frustum );

	return ss.str();
}

RenderBase::~RenderBase( void )
{
	DeleteBufferObject( GL_ARRAY_BUFFER, apiHandles[ 0 ] );
	DeleteBufferObject( GL_ELEMENT_ARRAY_BUFFER, apiHandles[ 1 ] );
}

//--------------------------------------------------------------
// BSPRenderer
//--------------------------------------------------------------
BSPRenderer::BSPRenderer( float viewWidth, float viewHeight, Q3BspMap& map_ )
	:
		RenderBase( map_ ), // Parent class

		glEffects( {
			{
				"tcModTurb",
				[]( const Program& p, const effect_t& e ) -> void
				{
					float turb = DEFORM_CALC_TABLE(
					deformCache.sinTable,
					0,
					e.data.wave.phase,
					GetTimeSeconds(),
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
					p.LoadMat2( "texRotate",
						&e.data.rotation2D.transform[ 0 ][ 0 ] );
					p.LoadVec2( "texCenter", e.data.rotation2D.center );
				}
			}
		} ),
		currLeaf( nullptr ),

		deltaTime( 0.0f ),
		frameTime( 0.0f ),
		targetFPS( 0.0f ),
		alwaysWriteDepth( false ),
		allowFaceCulling( true ),
		camera( new InputCamera() ),
		curView( VIEW_MAIN )
{
	camera->moveStep = 1.0f;
	camera->SetPerspective( 65.0f, viewWidth, viewHeight, G_STATIC_NEAR_PLANE,
		G_STATIC_FAR_PLANE );
}

BSPRenderer::~BSPRenderer( void )
{
}

// -------------------------------
// Meta
// -------------------------------

std::string BSPRenderer::GetBinLayoutString( void ) const
{
	std::stringstream ss;

	std::string first = RenderBase::GetBinLayoutString();

	ss << first;

	ss << "[BSPRenderer]\n";
	ss << SSTREAM_BYTE_OFFSET( BSPRenderer, textures );
	ss << SSTREAM_BYTE_OFFSET( BSPRenderer, glFaces );
	ss << SSTREAM_BYTE_OFFSET( BSPRenderer, glDebugFaces );
	ss << SSTREAM_BYTE_OFFSET( BSPRenderer, glEffects );
	ss << SSTREAM_BYTE_OFFSET( BSPRenderer, currLeaf );
	ss << SSTREAM_BYTE_OFFSET( BSPRenderer, deltaTime );
	ss << SSTREAM_BYTE_OFFSET( BSPRenderer, frameTime );
	ss << SSTREAM_BYTE_OFFSET( BSPRenderer, targetFPS );
	ss << SSTREAM_BYTE_OFFSET( BSPRenderer, alwaysWriteDepth );
	ss << SSTREAM_BYTE_OFFSET( BSPRenderer, camera );
	ss << SSTREAM_BYTE_OFFSET( BSPRenderer, curView );

	return ss.str();
}

// -------------------------------
// Init
// -------------------------------

void BSPRenderer::Prep( void )
{
	GEnableDepthBuffer();

	GL_CHECK( glEnable( GL_BLEND ) );

	GL_CHECK( glClearColor( 0.0f, 0.0f, 0.0f, 0.0f ) );

	GL_CHECK( glGenBuffers( apiHandles.size(), &apiHandles[ 0 ] ) );

	// Load main shader glPrograms
	{
		std::vector< std::string > attribs =
		{
			"position",
			"color",
			"tex0",
			"lightmap"
		};

		std::vector< std::string > uniforms =
		{
			"modelToView",
			"viewToClip",

			"mainImageSampler",
			"mainImageImageTransform",
			"mainImageImageScaleRatio",

			"lightmapSampler",
			"lightmapImageTransform",
			"lightmapImageScaleRatio"
		};

		MakeProg( "main", GMakeMainVertexShader(), GMakeMainFragmentShader(),
			uniforms, attribs );
	}
}

static void AddWhiteImage( gla_atlas_ptr_t& atlas )
{
	std::vector< uint8_t > whiteImage(
		BSP_LIGHTMAP_WIDTH * BSP_LIGHTMAP_HEIGHT * 4,
		0xFF
	);

	gla::push_atlas_image(
		*atlas,
		&whiteImage[ 0 ],
		BSP_LIGHTMAP_WIDTH,
		BSP_LIGHTMAP_HEIGHT,
		4,
		false
	);
}

void BSPRenderer::Load( renderPayload_t& payload )
{
	Prep();

	textures = std::move( payload.textureData );

	GLint oldAlign;
	GL_CHECK( glGetIntegerv( GL_UNPACK_ALIGNMENT, &oldAlign ) );
	GL_CHECK( glPixelStorei( GL_UNPACK_ALIGNMENT, 1 ) );

	// White images for both the shaders and main atlasses
	// are dummy fallbacks for erronous image indices / invalid image paths
	AddWhiteImage( textures[ TEXTURE_ATLAS_SHADERS ] );
	gla::gen_atlas_layers( *( textures[ TEXTURE_ATLAS_SHADERS ] ) );
	textures[ TEXTURE_ATLAS_SHADERS ]->default_image =
		textures[ TEXTURE_ATLAS_SHADERS ]->num_images - 1;

	AddWhiteImage( textures[ TEXTURE_ATLAS_MAIN ] );
	gla::gen_atlas_layers( *( textures[ TEXTURE_ATLAS_MAIN ] ) );
	textures[ TEXTURE_ATLAS_MAIN ]->default_image =
		textures[ TEXTURE_ATLAS_MAIN ]->num_images - 1;

	// Iterate through lightmaps and generate corresponding
	// texture data
	for ( bspLightmap_t& lightmap: map.data.lightmaps )
	{
		gla::push_atlas_image(
			*( textures[ TEXTURE_ATLAS_LIGHTMAPS ] ),
			&lightmap.map[ 0 ][ 0 ][ 0 ],
			BSP_LIGHTMAP_WIDTH,
			BSP_LIGHTMAP_HEIGHT,
			3,
			false
		);
	}

	// Sometimes a white image is explicitly desired for certain shader passes;
	// this will also serve as a fallback if needed.
	AddWhiteImage( textures[ TEXTURE_ATLAS_LIGHTMAPS ] );
	gla::gen_atlas_layers( *( textures[ TEXTURE_ATLAS_LIGHTMAPS ] ) );
	textures[ TEXTURE_ATLAS_LIGHTMAPS ]->default_image =
		textures[ TEXTURE_ATLAS_LIGHTMAPS ]->num_images - 1;

	GL_CHECK( glPixelStorei( GL_UNPACK_ALIGNMENT, oldAlign ) );

	camera->SetViewOrigin( map.GetFirstSpawnPoint().origin );

	LoadVertexData();

	// Basic program setup
	for ( const auto& iShader: map.effectShaders )
	{
		for ( const shaderStage_t& stage: iShader.second.stageBuffer )
		{
			stage.GetProgram().LoadMat4( "viewToClip",
				camera->ViewData().clipTransform );
		}
	}

	printf( "Program Count: %i\n", GNumPrograms() );

	glPrograms[ "main" ]->LoadMat4( "viewToClip",
		camera->ViewData().clipTransform );

	//MLOG_INFO( "%s", GetBinLayoutString().c_str() );

	GPrintContextInfo();
}

void BSPRenderer::LoadVertexData( void )
{
	glFaces.resize( map.data.numFaces );

	if ( gConfig.debugRender )
		glDebugFaces.resize( map.data.numFaces );

	std::vector< bspVertex_t > vertexData(
		&map.data.vertexes[ 0 ],
		&map.data.vertexes[ map.data.numVertexes ]
	);

#if G_STREAM_INDEX_VALUES
	size_t iboSize = 0;
#else
	std::vector< uint32_t > indexData;
	MapModelGenIndexBuffer( indexData );
#endif

	// cache the data already used for any polygon or mesh faces, so we don't have to
	// iterate through their index/vertex mapping every frame. For faces
	// which aren't of these two categories, we leave them be.
	for ( int32_t i = 0; i < map.data.numFaces; ++i )
	{
		const bspFace_t* face = &map.data.faces[ i ];

		if ( face->type == BSP_FACE_TYPE_PATCH )
		{
			glFaces[ i ].reset( new mapPatch_t() );
		}
		else
		{
			glFaces[ i ].reset( new mapModel_t() );
		}

		glFaces[ i ]->Generate( vertexData, &map, i );
		glFaces[ i ]->CalcBounds( map.data );

#if G_STREAM_INDEX_VALUES
		// Allocate the largest index buffer out of all models, so we can just
		// stream each item, and save GPU mallocs
		if ( iboSize < glFaces[ i ]->iboRange )
		{
			iboSize = glFaces[ i ]->iboRange;
		}
#endif
		if ( gConfig.debugRender )
		{
			MLOG_ASSERT( false, "gConfig.debugRender is true; you need to add the"\
				" vertex data to the glDebugFaces member" );
			std::random_device r;
			std::default_random_engine e( r() );
			std::uniform_real_distribution< float > urd( 0.0f, 1.0f );

			glm::vec4 color( urd( e ), urd( e ), urd( e ), 1.0f );

			glDebugFaces[ i ].color = color;
		}
	}

	// Allocate vertex data from map and store it all in a single vbo;
	// we use dynamic draw as a hint, considering that vertex deforms
	// require a buffer update
	GL_CHECK( glBindBuffer( GL_ARRAY_BUFFER, apiHandles[ 0 ] ) );
	GL_CHECK( glBufferData( GL_ARRAY_BUFFER, sizeof( vertexData[ 0 ] )
		* vertexData.size(), &vertexData[ 0 ], GL_DYNAMIC_DRAW ) );

#if !G_STREAM_INDEX_VALUES
	GL_CHECK( glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, apiHandles[ 1 ] ) );
	GL_CHECK( glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( indexData[ 0 ] )
		* indexData.size(), &indexData[ 0 ], GL_STATIC_DRAW ) );
#endif
}

// -------------------------------
// Frame
// -------------------------------

void BSPRenderer::Render( void )
{
	float startTime = GetTimeSeconds();

	if ( map.IsAllocated() )
	{
		RenderPass( camera->ViewData() );
	}

	frameTime = GetTimeSeconds() - startTime;

	frameCount++;
}

void BSPRenderer::Update( float dt )
{
	float fpsScalar = dt / ( targetFPS == 0.0f ? 1.0f : targetFPS );

	camera->Update( fpsScalar );

	viewParams_t& view = camera->ViewDataMut();

//	SetNearFar(
//		view.clipTransform,
//		G_STATIC_NEAR_PLANE,
//		G_STATIC_FAR_PLANE
//	);

	frustum->Update( view, false );

	deltaTime = dt;
}

// -------------------------------
// Rendering
// -------------------------------

void BSPRenderer::DrawMapPass(
	int32_t textureIndex,
	int32_t lightmapIndex,
	std::function< void( const Program& mainRef ) > callback
)
{
	GL_CHECK( glDepthFunc( GL_LEQUAL ) );
	GL_CHECK( glBlendFunc( GL_ONE, GL_ZERO ) );

	const Program& main = *( glPrograms.at( "main" ) );

	main.LoadDefaultAttribProfiles();

	if ( textureIndex < 0 ) // default to white image if nothing else
	{
		BindTexture(
			main,
			textures[ TEXTURE_ATLAS_LIGHTMAPS ],
			textures[ TEXTURE_ATLAS_LIGHTMAPS ]->num_images - 1,
			"mainImage",
			0
		);
	}
	else // key_image textureIndex
	{
		BindTexture(
			main,
			textures[ TEXTURE_ATLAS_MAIN ],
			textures[ TEXTURE_ATLAS_MAIN ]->key_image( textureIndex ),
			"mainImage",
			0
		);
	}

	if ( lightmapIndex < 0 )
	{
		BindTexture(
			main,
			textures[ TEXTURE_ATLAS_LIGHTMAPS ],
			textures[ TEXTURE_ATLAS_LIGHTMAPS ]->num_images - 1,
			"lightmap",
			1
		);
	}
	else
	{
		BindTexture(
			main,
			textures[ TEXTURE_ATLAS_LIGHTMAPS ],
			lightmapIndex,
			"lightmap",
			1
		);
	}

	main.LoadMat4( "modelToView", camera->ViewData().transform );

	main.Bind();

	callback( main );

	main.Release();

	// Even if the lightmaps atlas is used in place of the main atlas
	// (in the case of textureIndex == -1), calling release_from_active_slot()
	// using the main atlas pointer will still do what it's supposed to do:
	// activate texture slot 0 and unbind its currently bound texture.
	textures[ TEXTURE_ATLAS_MAIN ]->release_from_active_slot( 0 );
	textures[ TEXTURE_ATLAS_LIGHTMAPS ]->release_from_active_slot( 1 );
}

void BSPRenderer::DrawEffectPass( const drawTuple_t& data, drawCall_t callback )
{
	const shaderInfo_t* shader = std::get< 1 >( data );
	int lightmapIndex = std::get< 3 >( data );
	bool isSolid = std::get< 4 >( data );

	// Each effect pass is allowed only one texture,
	// so we don't need a second texcoord
	GL_CHECK( glDisableVertexAttribArray( 3 ) );

	// Assess the current culling situation; if the current
	// shader uses a setting which differs from what's currently set,
	// we restore our cull settings to their previous values after this draw
	GLint oldCull = -1, oldCullMode = 0, oldFrontFace = 0;

	if  ( allowFaceCulling )
	{
		GL_CHECK( glGetIntegerv( GL_CULL_FACE, &oldCull ) );

		// Store values right now, before the potential change in state
		if ( oldCull )
		{
			GL_CHECK( glGetIntegerv( GL_FRONT_FACE, &oldFrontFace ) );
			GL_CHECK( glGetIntegerv( GL_CULL_FACE_MODE, &oldCullMode ) );
		}

		// Check for desired face culling
		if ( shader->cullFace == GL_NONE )
		{
			GL_CHECK( glDisable( GL_CULL_FACE ) );
		}
		else
		{
			if ( !oldCull ) // Not enabled, so we need to activate it
				GL_CHECK( glEnable( GL_CULL_FACE ) );

			GL_CHECK( glCullFace( shader->cullFace ) );
		}
	}

	if ( alwaysWriteDepth )
	{
		GL_CHECK( glDepthMask( GL_TRUE ) );
	}

	// Set to true to log info after function leaves
	logEffectPass_t< false > logger( shader );

	// Make sure this we have depth func set to LEQUAL before we return.
	GLenum lastDepth = GL_LEQUAL;

	// Used primarily for texture scrolling.
	glm::vec2 timeScalarSeconds( GetTimeSeconds() );

	for ( int32_t i = 0; i < shader->stageCount; ++i )
	{
		const shaderStage_t& stage = shader->stageBuffer[ i ];
		const Program& stageProg = stage.GetProgram();

		logger.Push( i, stage );

		stageProg.LoadMat4( "modelToView", camera->ViewData().transform );

		GL_CHECK( glBlendFunc( stage.blendSrc, stage.blendDest ) );
		GL_CHECK( glDepthFunc( stage.depthFunc ) );
		lastDepth = stage.depthFunc;

		if ( !alwaysWriteDepth )
		{
			if ( isSolid || stage.depthPass )
			{
				GL_CHECK( glDepthMask( GL_TRUE ) );
			}
			else
			{
				GL_CHECK( glDepthMask( GL_FALSE ) );
			}
		}

		gla_atlas_ptr_t* atlas = nullptr;
		int32_t texIndex = -1;

		if ( stage.mapType == MAP_TYPE_IMAGE )
		{
			atlas = &textures[ TEXTURE_ATLAS_SHADERS ];
			texIndex = stage.textureIndex;
		}
		else if ( stage.mapType == MAP_TYPE_WHITE_IMAGE || lightmapIndex < 0 )
		{
			atlas = &textures[ TEXTURE_ATLAS_LIGHTMAPS ];
			texIndex = textures[ TEXTURE_ATLAS_LIGHTMAPS ]->num_images - 1;
		}
		else
		{
			atlas = &textures[ TEXTURE_ATLAS_LIGHTMAPS ];
			texIndex = lightmapIndex;
		}

		if ( texIndex < 0 )
		{
			MLOG_INFO_ONCE( "Zero Found for %s:[%i]%s", &shader->name[ 0 ], i, &stage.texturePath[ 0 ] );
		//	return;
		}

		BindTexture(
			stageProg,
			*atlas,
			texIndex,
			nullptr,
			0
		);

		for ( effect_t e: stage.effects )
		{
			if ( e.name == "tcModScroll" )
			{
				e.data.xyzw[ 2 ] = timeScalarSeconds.x;
				e.data.xyzw[ 3 ] = timeScalarSeconds.y;
			}
			else if ( e.name == "tcModRotate" )
			{
				e.data.rotation2D.center[ 0 ] = 0.5f;
				e.data.rotation2D.center[ 1 ] = 0.5f;
			}

			glEffects.at( e.name )( stageProg, e );
		}

		stageProg.LoadDefaultAttribProfiles();

#ifdef DEBUG
		if ( GHasBadProgram() )
		{
			GPrintBadProgram();
		}
#endif

		stageProg.Bind();
		callback( std::get< 0 >( data ), stageProg, &stage );
		stageProg.Release();

		( *atlas )->release_from_active_slot( 0 );
	}

	// No need to change state here unless there's the possibility
	// we've modified it
	if ( !alwaysWriteDepth )
	{
		GL_CHECK( glDepthMask( GL_TRUE ) );
	}

	if ( lastDepth != GL_LEQUAL )
	{
		GL_CHECK( glDepthFunc( GL_LEQUAL ) );
	}

	GL_CHECK( glEnableVertexAttribArray( 3 ) );

	// Did we bother checking earlier?
	if ( allowFaceCulling && oldCull != -1 )
	{
		// If true, we had culling enabled previously, so
		// restore previous settings; otherwise, we ensure it's disabled
		if ( oldCull == GL_TRUE )
		{
			GL_CHECK( glEnable( GL_CULL_FACE ) );
			GL_CHECK( glCullFace( oldCullMode ) );
			GL_CHECK( glFrontFace( oldFrontFace ) );
		}
		else
		{
			GL_CHECK( glDisable( GL_CULL_FACE ) );
		}
	}
}

void BSPRenderer::DrawFaceVerts( const drawPass_t& pass,
	const shaderStage_t* stage ) const
{
	UNUSED( stage );

	const mapModel_t& m = *( glFaces[ pass.faceIndex ] );

	if ( pass.shader && pass.shader->deform )
	{
		DeformVertexes( m, pass.shader );
	}

	if ( pass.face->type == BSP_FACE_TYPE_POLYGON
		|| pass.face->type == BSP_FACE_TYPE_MESH )
	{
		GU_DrawElements( GL_TRIANGLES, m.iboOffset, m.iboRange );
	}
	else if ( pass.face->type == BSP_FACE_TYPE_PATCH )
	{
		const mapPatch_t& p = *( m.ToPatch() );
		GU_MultiDrawElements( GL_TRIANGLE_STRIP, p.rowIndices, p.trisPerRow );
	}
}

// -------------------------------
// State management
// -------------------------------

void BSPRenderer::BindTexture(
	const Program& program,
	const gla_atlas_ptr_t& atlas,
	uint16_t image,
	const char* prefix,
	int offset
)
{
	gla::atlas_image_info_t imageData = atlas->image_info( image );

	atlas->bind_to_active_slot( imageData.layer, offset );

	glm::vec4 transform(
		imageData.coords.x * imageData.inverse_layer_dims.x,
		imageData.coords.y * imageData.inverse_layer_dims.y,
		atlas->dims_x[ image ],
		atlas->dims_y[ image ]
	);

	if ( prefix )
	{
		std::string strfix( prefix );

		program.LoadInt( strfix + "Sampler", offset );
		program.LoadVec2( strfix + "ImageScaleRatio",
			imageData.inverse_layer_dims );
		program.LoadVec4( strfix + "ImageTransform", transform );
	}
	else
	{
		program.LoadInt( "sampler0", offset );
		program.LoadVec2( "imageScaleRatio", imageData.inverse_layer_dims );
		program.LoadVec4( "imageTransform", transform );
	}
}

void BSPRenderer::LoadLightVol(
	const drawPass_t& pass,
	const Program& prog
) const
{
	if ( pass.lightvol )
	{
		float phi = glm::radians(
			( float ) pass.lightvol->direction.x * 4.0f
		);

		float theta = glm::radians(
			( float ) pass.lightvol->direction.y * 4.0f
		);

		glm::vec3 dirToLight(
			glm::cos( theta ) * glm::cos( phi ),
			glm::sin( phi ),
			glm::cos( phi ) * glm::sin( theta )
		);

		glm::vec3 ambient( pass.lightvol->ambient );
		ambient *= Inv255< float >();

		glm::vec3 directional( pass.lightvol->directional );
		directional *= Inv255< float >();

		prog.LoadVec3( "fragDirToLight", dirToLight );
		prog.LoadVec3( "fragAmbient", ambient );
		prog.LoadVec3( "fragDirectional", directional );
	}
}

void BSPRenderer::DeformVertexes( const mapModel_t& m,
	const shaderInfo_t* shader ) const
{
	if ( !shader || shader->deformCmd == VERTEXDEFORM_CMD_UNDEFINED ) return;

	std::vector< bspVertex_t > verts = m.clientVertices;

	for ( uint32_t i = 0; i < verts.size(); ++i )
	{
		glm::vec3 n(
			verts[ i ].normal * GenDeformScale( verts[ i ].position, shader )
		);
		verts[ i ].position += n;
	}

	UpdateBufferObject< bspVertex_t >(
		GL_ARRAY_BUFFER,
		apiHandles[ 0 ],
		m.vboOffset,
		verts,
		false
	);
}

// -------------------------------
// BSP Traversal
// -------------------------------

static bool SortOpaqueFacePredicate( const drawFace_t& a, const drawFace_t& b )
{
	return a.sort < b.sort;
}

static bool SortTransparentFacePredicate( const drawFace_t& a, const drawFace_t& b )
{
	return a.sort > b.sort;
}

void BSPRenderer::RenderPass( const viewParams_t& view )
{
	memset( &gCounts, 0, sizeof( gCounts ) );

	drawPass_t pass( map, view );
	pass.leaf = map.FindClosestLeaf( pass.view.origin );

	frustum->Update( pass.view, true );

	pass.facesVisited.assign( pass.facesVisited.size(), 0 );

	// We start at index 1 because the 0th index
	// provides a model which represents the entire map.
	for ( int32_t i = 1; i < map.data.numModels; ++i )
	{
		bspModel_t* model = &map.data.models[ i ];

		AABB bounds( model->boxMax, model->boxMin );

		if ( !frustum->IntersectsBox( bounds ) )
		{
			continue;
		}

		pass.isSolid = true;
		for ( int32_t j = 0; j < model->numFaces; ++j )
			ProcessFace( pass, model->faceOffset + j );

		pass.isSolid = false;
		for ( int32_t j = 0; j < model->numFaces; ++j )
			ProcessFace( pass, model->faceOffset + j );
	}

	pass.type = PASS_DRAW;

	if ( allowFaceCulling )
	{
		GL_CHECK( glEnable( GL_CULL_FACE ) );
		GL_CHECK( glCullFace( GL_FRONT ) );
		GL_CHECK( glFrontFace( GL_CCW ) );
	}
	else
	{
		GL_CHECK( glDisable( GL_CULL_FACE ) );
	}

	TraverseDraw( pass, true );
	TraverseDraw( pass, false );

	// Sort the faces and draw them.

	std::sort( pass.opaqueFaces.begin(), pass.opaqueFaces.end(), SortOpaqueFacePredicate );
	DrawFaceList( pass, true );

	std::sort( pass.transparentFaces.begin(), pass.transparentFaces.end(), SortTransparentFacePredicate );
	DrawFaceList( pass, false );

	if ( allowFaceCulling )
	{
		GL_CHECK( glDisable( GL_CULL_FACE ) );
	}
}

void BSPRenderer::ProcessFace( drawPass_t& pass, uint32_t index )
{
	// if pass.facesVisited[ faceIndex ] is still false after this criteria's
	// evaluations, we'll pick it up on the next pass as it will meet
	// the necessary criteria then.
	if ( pass.facesVisited[ index ] )
	{
		return;
	}

	pass.face = &map.data.faces[ index ];
	pass.faceIndex = index;
	pass.shader = map.GetShaderInfo( index );

	if ( map.IsNoDrawShader( pass.shader ) )
	{
		// evaluating this on the next pass will be faster than
		// the function call.
		pass.facesVisited[ index ] = true; 
		return;
	}

	bool transparent = map.IsTransparentShader( pass.shader );

	bool add = ( !pass.isSolid && transparent ) || ( pass.isSolid && !transparent );

	if ( add )
	{
		drawFace_t dface;

		dface.SetTransparent( transparent );
		dface.SetMapFaceIndex( index );
		dface.SetShaderListIndex( pass.shader->sortListIndex );

		// TODO: do view-space zDepth evaluation here.
		
		// Keep track of max/min view-space z-values for each face;
		// ensure that closest point on face bounds (out of the 8 corners)
		// relative to the view frustum is compared against max-z and farthest
		// relative to the view frustum is compared against min-z. 

		// This is 8 matrix/vector multiplies (using the world->camera transform). Using SIMD and packing
		// corner vectors into a 4D matrix you can do these ops in two.
		// Get it working first, though.

		if ( transparent )
		{
			pass.transparentFaces.push_back( dface );
		}
		else
		{
			pass.opaqueFaces.push_back( dface );
		}
	}
}

void BSPRenderer::DrawNode( drawPass_t& pass, int32_t nodeIndex )
{
	if ( nodeIndex < 0 )
	{
		pass.viewLeafIndex = -( nodeIndex + 1 );
		const bspLeaf_t* viewLeaf = &map.data.leaves[ pass.viewLeafIndex ];

		if ( !map.IsClusterVisible( pass.leaf->clusterIndex,
				viewLeaf->clusterIndex ) )
		{
			return;
		}

		AABB leafBounds;

		leafBounds.maxPoint = glm::vec3(
			viewLeaf->boxMax.x,
			viewLeaf->boxMax.y,
			viewLeaf->boxMax.z
		);

		leafBounds.minPoint = glm::vec3(
			viewLeaf->boxMin.x,
			viewLeaf->boxMin.y,
			viewLeaf->boxMin.z
		);

		if ( !frustum->IntersectsBox( leafBounds ) )
		{
			return;
		}

		for ( int32_t i = 0; i < viewLeaf->numLeafFaces; ++i )
		{
			ProcessFace( pass, 
				map.data.leafFaces[ viewLeaf->leafFaceOffset + i ].index );
		}
	}
	else
	{
		const bspNode_t* const node = &map.data.nodes[ nodeIndex ];
		const bspPlane_t* const plane = &map.data.planes[ node->plane ];

		float d = glm::dot( pass.view.origin, glm::vec3( plane->normal.x,
			plane->normal.y, plane->normal.z ) );

		// We're in front of the plane if d > plane->distance.
		// If both of these are true, it makes sense to draw what is in
		// front of us, as any non-solid object can be handled properly by
		// depth if it's infront of the partition plane and we're behind it

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


void BSPRenderer::TraverseDraw( drawPass_t& pass, bool solid )
{
	pass.isSolid = solid;
	DrawNode( pass, 0 );
}

void BSPRenderer::DrawFaceList( drawPass_t& pass, bool solid )
{
	const std::vector< drawFace_t >& faceList = solid ? pass.opaqueFaces : pass.transparentFaces;
	const shaderList_t& sortedShaderList = solid ? map.opaqueShaderList : map.transparentShaderList;

	pass.isSolid = solid;

	for ( size_t i = 0; i < faceList.size(); ++i )
	{
		pass.shader = sortedShaderList[ faceList[ i ].GetShaderListIndex() ];
		pass.faceIndex = faceList[ i ].GetMapFaceIndex();
		pass.face = &map.data.faces[ pass.faceIndex ];


		if ( map.IsDefaultShader( pass.shader ) )
		{
			pass.drawType = PASS_DRAW_MAIN;
		}
		else
		{
			pass.drawType = PASS_DRAW_EFFECT;
		}

		DrawFace( pass );
	}
}

void BSPRenderer::DrawFace( drawPass_t& pass )
{
	GL_CHECK( glBlendFunc( GL_ONE, GL_ZERO ) );
	GL_CHECK( glDepthFunc( GL_LEQUAL ) );

	auto LEffectCallback = [ &pass, this ]( const void* param, const Program& prog,
		const shaderStage_t* stage )
	{
		UNUSED( param );
		UNUSED( prog );
		DrawFaceVerts( pass, stage );
	};

	auto LNonEffectCallback = [ &pass, this ]( const Program& prog )
	{
		UNUSED( prog );
		DrawFaceVerts( pass, nullptr );
	};

	switch ( pass.drawType )
	{
		case PASS_DRAW_EFFECT:
		{
			drawTuple_t data = std::make_tuple(
				nullptr,
				pass.shader,
				pass.face->shader,
				pass.face->lightmapIndex,
				pass.isSolid
			);

			DrawEffectPass( data, LEffectCallback );
		}
			break;
		default:
		case PASS_DRAW_MAIN:

			DrawMapPass( pass.face->shader, pass.face->lightmapIndex, LNonEffectCallback );

			break;
	}
}






/*
int BSPRenderer::CalcLightvolIndex( const drawPass_t& pass ) const
{
	const glm::vec3& max = map.data.models[ 0 ].boxMax;
	const glm::vec3& min = map.data.models[ 0 ].boxMin;

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

	return ( dindex.z * dims.x * dims.y + dims.x * dindex.y + dindex.x ) % map.data.numLightvols;
}
*/
