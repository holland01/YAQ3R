#include "renderer.h"
#include "shader.h"
#include "io.h"
#include "math_util.h"
#include "effect_shader.h"
#include "deform.h"
#include <glm/gtx/string_cast.hpp>
#include <fstream>

struct config_t
{
	bool drawFacesOnly: 1;
	bool drawIrradiance: 1;
	bool drawFacePatches: 1;
	bool drawFaceBounds: 1;
	bool drawAtlasTextureBoxes: 1;
	bool logStageTexCoordData: 1;
};

static config_t gConfig =
{
	false,
	false,
	false,
	false,
	false,
	false
};

static uint32_t gWriteCount = 0;

static std::unique_ptr< std::fstream > gRenderLogger( nullptr );

static inline void WriteLog( std::stringstream& out )
{
#ifdef DEBUG
	if ( gConfig.logStageTexCoordData && !gRenderLogger )
	{
		gRenderLogger.reset( new std::fstream( "log/renderer.log", std::ios_base::trunc | std::ios_base::out ) );

		if ( !gRenderLogger->good() )
			MLOG_ERROR( "Could not open logger filepath." );
	}

	if ( gConfig.logStageTexCoordData && gRenderLogger && gWriteCount++ < 30 )
		*gRenderLogger << out.rdbuf();
#else
	UNUSED( out );
#endif
}

#define CALC_LIGHTMAP_INDEX( index ) ( map->data.numTextures + ( index ) )

static uint64_t frameCount = 0;

static INLINE void AddSurfaceData( drawSurface_t& surf, int faceIndex, std::vector< mapModel_t >& glFaces )
{
	mapModel_t& model = glFaces[ faceIndex ];

	if ( surf.faceType == BSP_FACE_TYPE_PATCH )
	{
		surf.bufferOffsets.insert( surf.bufferOffsets.end(), model.rowIndices.begin(), model.rowIndices.end() );
		surf.bufferRanges.insert( surf.bufferRanges.end(), model.trisPerRow.begin(), model.trisPerRow.end() );
	}
	else
	{
		surf.bufferOffsets.push_back( model.iboOffset );
		surf.bufferRanges.push_back( model.iboRange );
	}

	if ( surf.shader
	&& ( /*!!( surf.shader->surfaceParms & SURFPARM_ENVMAP ) ||*/ surf.shader->deform ) )
	{
		surf.faceIndices.push_back( faceIndex );
	}
}

//--------------------------------------------------------------
mapModel_t::mapModel_t( void )
	: deform( false ),
	  vboOffset( 0 ),
	  subdivLevel( 0 )
{
}

mapModel_t::~mapModel_t( void )
{
}

void mapModel_t::CalcBounds( const std::vector< int32_t >& indices, int32_t faceType, const mapData_t& data )
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
	:   mainSampler( { G_UNSPECIFIED } ),
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
					p.LoadMat2( "texRotate", &e.data.rotation2D.transform[ 0 ][ 0 ] );
					p.LoadVec2( "texCenter", e.data.rotation2D.center );
				}
			}
		} ),
		currLeaf( nullptr ),
		apiHandles( {{ 0, 0 }} ),
		deltaTime( 0.0f ),
		frameTime( 0.0f ),
		map ( new Q3BspMap() ),
		camera( nullptr ),
		frustum( new Frustum() ),
		curView( VIEW_MAIN )
{
	viewParams_t view;
	view.origin = glm::vec3( -131.291901f, -61.794476f, -163.203659f ); /// debug position which doesn't kill framerate

	camera = new InputCamera( view, EuAng() );
	camera->SetPerspective( 45.0f, viewWidth, viewHeight, 0.1f, 5000.0f );
}

BSPRenderer::~BSPRenderer( void )
{
	DeleteBufferObject( GL_ARRAY_BUFFER, apiHandles[ 0 ] );
	DeleteBufferObject( GL_ELEMENT_ARRAY_BUFFER, apiHandles[ 1 ] );

	delete map;
	delete frustum;
	delete camera;
}

void BSPRenderer::MakeProg( const std::string& name, const std::string& vertPath, const std::string& fragPath,
		const std::vector< std::string >& uniforms, const std::vector< std::string >& attribs )
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

	glPrograms[ name ] = std::unique_ptr< Program >( new Program( vertex, fragment, uniforms, attribs ) );
}

void BSPRenderer::Prep( void )
{
	GL_CHECK( glEnable( GL_DEPTH_TEST ) );
	GL_CHECK( glEnable( GL_BLEND ) );

	GL_CHECK( glDepthFunc( GL_LEQUAL ) );

	GL_CHECK( glClearColor( 1.0f, 1.0f, 1.0f, 1.0f ) );

	GU_ClearDepth( 1.0f );

	GL_CHECK( glGenBuffers( apiHandles.size(), &apiHandles[ 0 ] ) );

	GL_CHECK( glDisable( GL_CULL_FACE ) );

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
			"mainImageActive",

			"lightmapSampler",
			"lightmapImageTransform",
			"lightmapImageScaleRatio",
			"lightmapActive"
		};

		MakeProg( "main", "src/main_es.vert", "src/main_es.frag", uniforms, attribs );
	}
}

bool BSPRenderer::IsTransFace( int32_t faceIndex, const shaderInfo_t* shader ) const
{
	const bspFace_t* face = &map->data.faces[ faceIndex ];

	if ( face && face->texture != -1 )
	{
		if ( shader )
		{
			return ( shader->surfaceParms & SURFPARM_TRANS ) != 0;
		}
		MLOG_WARNING( "BSPRenderer::IsTransFace was changed - results involving alpha channel usage may be different!" );
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

void BSPRenderer::Load( const std::string& filepath )
{
	MLOG_INFO( "Loading file %s....\n", filepath.c_str() );

	map->Read( filepath, 1 );

	if ( mainSampler.id == G_UNSPECIFIED )
		mainSampler = GMakeSampler();

	GLint oldAlign;
	GL_CHECK( glGetIntegerv( GL_UNPACK_ALIGNMENT, &oldAlign ) );
	GL_CHECK( glPixelStorei( GL_UNPACK_ALIGNMENT, 1 ) );

	{
		gImageParamList_t shaderTextures;
		S_LoadShaders( &map->data, mainSampler, shaderTextures, map->effectShaders );
		gTextureMakeParams_t makeParams( shaderTextures, mainSampler );
		shaderTexHandle = GMakeTexture( makeParams, 0 );
	}

	LoadMainImages();
	LoadLightmaps();

	GL_CHECK( glPixelStorei( GL_UNPACK_ALIGNMENT, oldAlign ) );

	LoadVertexData();

	// Basic program setup
	for ( const auto& iShader: map->effectShaders )
		for ( const shaderStage_t& stage: iShader.second.stageBuffer )
			stage.program->LoadMat4( "viewToClip", camera->ViewData().clipTransform );

	glPrograms[ "main" ]->LoadMat4( "viewToClip", camera->ViewData().clipTransform );
}

void BSPRenderer::LoadMainImages( void )
{
	//---------------------------------------------------------------------
	// Load Textures:
	// This is just a temporary hack to brute force load assets without taking into account the effect shader files.
	// Now, we find and generate the textures. We first start with the image files.
	//---------------------------------------------------------------------

	const char* validImgExt[] =
	{
		".jpg", ".png", ".tga", ".tiff", ".bmp"
	};

	glTextures.resize( map->data.numTextures );

	for ( int32_t t = 0; t < map->data.numTextures; t++ )
	{
		glTextures[ t ].sampler = mainSampler;
		bool success = false;

		std::string fname( map->data.textures[ t ].name );

		const std::string& texPath = map->data.basePath + fname;

		// If we don't have a file extension appended in the name,
		// try to find one for it which is valid
		{
			for ( int32_t i = 0; i < SIGNED_LEN( validImgExt ); ++i )
			{
				const std::string& str = texPath + std::string( validImgExt[ i ] );

				if ( GLoadImageFromFile( str, glTextures[ t ] ) )
				{
					success = true;
					break;
				}
			}
		}

		// We stub the buffer with a simple dummy fill - otherwise, bad things will happen in the texture fetches.
		if ( !success )
		{
			GSetImageBuffer( glTextures[ t ], 32, 32, 255 );
			goto FAIL_WARN;
		}

		continue;

FAIL_WARN:
		MLOG_WARNING( "Could not find a file extension for \'%s\'", texPath.c_str() );
	}

	{
		gTextureMakeParams_t makeParams( glTextures, mainSampler );
		mainTexHandle = GMakeTexture( makeParams, 0 );
	}
}

void BSPRenderer::LoadLightmaps( void )
{
	gImageParamList_t lightmaps;

	// And then generate all of the lightmaps
	for ( int32_t l = 0; l < map->data.numLightmaps; ++l )
	{
		gImageParams_t image;
		image.sampler = mainSampler;
		GSetImageBuffer( image, BSP_LIGHTMAP_WIDTH, BSP_LIGHTMAP_HEIGHT, 255 );

		Pixels_To32Bit( &image.data[ 0 ],
			&map->data.lightmaps[ l ].map[ 0 ][ 0 ][ 0 ], 3, BSP_LIGHTMAP_WIDTH * BSP_LIGHTMAP_HEIGHT );

		lightmaps.push_back( image );
	}

	gTextureMakeParams_t makeParams( lightmaps, mainSampler );
	lightmapHandle = GMakeTexture( makeParams, 0 );
}

//---------------------------------------------------------------------
// Generate our face/render data
//---------------------------------------------------------------------
void BSPRenderer::LoadVertexData( void )
{
	glFaces.resize( map->data.numFaces );
	std::vector< bspVertex_t > vertexData( &map->data.vertexes[ 0 ], &map->data.vertexes[ map->data.numVertexes ] );

	std::vector< uint32_t > indexData;

	// cache the data already used for any polygon or mesh faces, so we don't have to iterate through their index/vertex mapping every frame. For faces
	// which aren't of these two categories, we leave them be.
	for ( int32_t i = 0; i < map->data.numFaces; ++i )
	{
		std::vector< int32_t > modIndices;
		const shaderInfo_t* shader = map->GetShaderInfo( i );
		mapModel_t* mod = &glFaces[ i ];

		const bspFace_t* face = map->data.faces + i;

		if ( face->type == BSP_FACE_TYPE_MESH || face->type == BSP_FACE_TYPE_POLYGON )
		{
			mod->iboRange = face->numMeshVertexes;
			mod->iboOffset = indexData.size();
			for ( int32_t j = 0; j < face->numMeshVertexes; ++j )
			{
				modIndices.push_back( face->vertexOffset + map->data.meshVertexes[ face->meshVertexOffset + j ].offset );
			}
		}
		else if ( face->type == BSP_FACE_TYPE_PATCH )
		{
			mod->vboOffset = ( GLuint ) vertexData.size();

			int width = ( face->size[ 0 ] - 1 ) / 2;
			int height = ( face->size[ 1 ] - 1 ) / 2;

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

					GenPatch( modIndices, mod, shader, baseDest, ( int32_t ) vertexData.size() );
				}
			}

			const uint32_t L1 = mod->subdivLevel + 1;
			mod->rowIndices.resize( width * height * mod->subdivLevel, 0 );
			mod->trisPerRow.resize( width * height * mod->subdivLevel, 0 );

			for ( size_t y = 0; y < mod->rowIndices.size(); ++y )
			{
				mod->trisPerRow[ y ] = 2 * L1;
				mod->rowIndices[ y ] = indexData.size() + y * 2 * L1;
			}

			vertexData.insert( vertexData.end(), mod->patchVertices.begin(), mod->patchVertices.end() );
		}

		indexData.insert( indexData.end(), modIndices.begin(), modIndices.end() );
		mod->CalcBounds( modIndices, face->type, map->data );
	}

	// Allocate vertex data from map and store it all in a single vbo
	GL_CHECK( glBindBuffer( GL_ARRAY_BUFFER, apiHandles[ 0 ] ) );
	GL_CHECK( glBufferData( GL_ARRAY_BUFFER, sizeof( vertexData[ 0 ] ) * vertexData.size(), &vertexData[ 0 ], GL_DYNAMIC_DRAW ) );

	GL_CHECK( glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, apiHandles[ 1 ] ) );
	GL_CHECK( glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( indexData[ 0 ] ) * indexData.size(), &indexData[ 0 ], GL_STATIC_DRAW ) );

}

void BSPRenderer::Render( void )
{
	float startTime = GetTimeSeconds();

	RenderPass( camera->ViewData(), false );

	frameTime = GetTimeSeconds() - startTime;

	frameCount++;
}

void BSPRenderer::RenderPass( const viewParams_t& view, bool envmap )
{
	GL_CHECK( glDisable( GL_CULL_FACE ) );
	GU_ClearDepth( 1.0f );
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

			if ( gConfig.drawFacesOnly )
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
			{
				continue;
			}

			LoadPassParams( pass, faceIndex, PASS_DRAW_MAIN );

			bool transparent = IsTransFace( pass.faceIndex, pass.shader );

			bool add = ( !pass.isSolid && transparent ) || ( pass.isSolid && !transparent );

			if ( add )
			{
				// Only draw individual faces if they're patches, since meshes and polygons
				// can be easily grouped together from the original vbo
				if ( ( pass.face->type == BSP_FACE_TYPE_PATCH && gConfig.drawFacePatches ) || gConfig.drawFacesOnly )
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

void BSPRenderer::DrawMapPass( int32_t textureIndex, int32_t lightmapIndex, std::function< void( const Program& mainRef ) > callback )
{
	const Program& main = *( glPrograms.at( "main" ) );

	main.LoadDefaultAttribProfiles();

	GU_SetupTexParams( main, "mainImage", mainTexHandle, textureIndex, 0 );
	GU_SetupTexParams( main, "lightmap", lightmapHandle, lightmapIndex, 1 );

	main.LoadMat4( "modelToView", camera->ViewData().transform );

	main.Bind();

	callback( main );

	main.Release();

	GReleaseTexture( mainTexHandle, 0 );
	GReleaseTexture( lightmapHandle, 1 );
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
		surfList.push_back( surf );
	}
}

void BSPRenderer::DrawSurface( const drawSurface_t& surf, const shaderStage_t* stage ) const
{
	for ( int32_t i: surf.faceIndices )
	{
		DeformVertexes( glFaces[ i ], surf.shader );
	}

	if ( stage && gConfig.logStageTexCoordData )
	{
		std::stringstream sstream;
		LogWriteAtlasTexture( sstream, shaderTexHandle, stage );
		WriteLog( sstream );
	}

	GLenum mode = ( surf.faceType == BSP_FACE_TYPE_PATCH )? GL_TRIANGLE_STRIP: GL_TRIANGLES;

	GU_MultiDrawElements( mode, surf.bufferOffsets, surf.bufferRanges );
}

void BSPRenderer::DrawEffectPass( const drawTuple_t& data, drawCall_t callback )
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
		const Program& stageProg = *( stage.program );

		stageProg.LoadMat4( "modelToView", camera->ViewData().transform );

		GL_CHECK( glBlendFunc( stage.blendSrc, stage.blendDest ) );
		GL_CHECK( glDepthFunc( stage.depthFunc ) );

		// TODO: use correct dimensions for texture
		glm::vec2 texDims( 64.0f );

		const gTextureHandle_t& handle = stage.mapType == MAP_TYPE_IMAGE? shaderTexHandle: lightmapHandle;
		const int32_t texIndex = ( stage.mapType == MAP_TYPE_IMAGE )? stage.textureIndex: lightmapIndex;

		GU_SetupTexParams( stageProg, nullptr, handle, texIndex, 0 );

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

		stageProg.LoadDefaultAttribProfiles();

		stageProg.Bind();
		callback( std::get< 0 >( data ), stageProg, &stage );
		stageProg.Release();

		GReleaseTexture( handle );
	}

	GL_CHECK( glEnableVertexAttribArray( 3 ) );

	GL_CHECK( glBlendFunc( GL_ONE, GL_ZERO ) );
	GL_CHECK( glDisable( GL_CULL_FACE ) );
	GL_CHECK( glDepthFunc( GL_LEQUAL ) );
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

		prog.LoadDefaultAttribProfiles();

		DrawSurface( surf, stage );
	};

	UNUSED( LEffectCallback );

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
				UNUSED( main );
				DrawSurface( surf, nullptr );
			});
		}
	}
}

void BSPRenderer::DrawFaceVerts( const drawPass_t& pass, const shaderStage_t* stage, const Program& program ) const
{
	UNUSED( stage );

	const mapModel_t& m = glFaces[ pass.faceIndex ];

	if ( pass.face->type == BSP_FACE_TYPE_POLYGON || pass.face->type == BSP_FACE_TYPE_MESH )
	{
		GL_CHECK( glDrawElements( GL_TRIANGLES, m.iboRange, GL_UNSIGNED_INT, ( const GLvoid* ) m.iboOffset ) );
	}
	else if ( pass.face->type == BSP_FACE_TYPE_PATCH )
	{
		if ( pass.shader && pass.shader->deform )
		{
			DeformVertexes( m, pass.shader );
		}

		program.LoadDefaultAttribProfiles();

		GU_MultiDrawElements( GL_TRIANGLE_STRIP, m.rowIndices, m.trisPerRow );
	}
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

	UpdateBufferObject< bspVertex_t >( GL_ARRAY_BUFFER, apiHandles[ 0 ], m.vboOffset, verts, false );
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
