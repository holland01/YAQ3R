#pragma once

#include "common.h"
#include "q3bsp.h"
#include "input.h"
#include "frustum.h"
#include "aabb.h"
#include "glutil.h"
#include "renderer/util.h"
#include <array>
#include <functional>
#include <cfloat>
#include <unordered_map>

// Draw flags
enum
{
	RENDER_BSP_LIGHTMAP_INFO = 1 << 0,
	RENDER_BSP_EFFECT = 1 << 1,
	RENDER_BSP_ALWAYS_POLYGON_OFFSET = 1 << 2,
	RENDER_BSP_USE_TCMOD = 1 << 3
};

enum passType_t
{
	PASS_DRAW,
	PASS_BRUSH
};

enum passDrawType_t
{
	PASS_DRAW_EFFECT = 0,
	PASS_DRAW_MAIN,
	PASS_DRAW_DEBUG
};

enum objectType_t
{
	OBJECT_FACE,
	OBJECT_SURFACE
};

enum viewMode_t
{
	VIEW_MAIN = 0,
	VIEW_LIGHT_SAMPLE,
};

using drawCall_t = std::function<
	void(
		const void* param,
		const Program& program,
		const shaderStage_t* stage
	)
>;

enum
{
	DRAWFACE_SORT_SHADER_INDEX_BITS = 8,
	DRAWFACE_SORT_SHADER_INDEX_SHIFT = 24,
	DRAWFACE_SORT_SHADER_INDEX_MASK = MAKE_BASE2_MASK( DRAWFACE_SORT_SHADER_INDEX_BITS, DRAWFACE_SORT_SHADER_INDEX_SHIFT ),

	DRAWFACE_SORT_DEPTH_SHIFT = 0,
	DRAWFACE_SORT_DEPTH_BITS = 24,
	DRAWFACE_SORT_DEPTH_MASK = MAKE_BASE2_MASK( DRAWFACE_SORT_DEPTH_BITS, DRAWFACE_SORT_DEPTH_SHIFT ),

	DRAWFACE_METADATA_IS_TRANSPARENT_SHIFT = 31,
	DRAWFACE_METADATA_MAP_FACE_INDEX_MASK = 0x7FFFFFFF

};

struct drawFace_t
{
	size_t sort = 0;			// z-depth, and shader sort index
	size_t metadata = 0;		// map face index, and whether or not the face is transparent 

	bool GetTransparent( void ) const 
	{ 
		return ( bool ) !!( metadata & ( 1 << DRAWFACE_METADATA_IS_TRANSPARENT_SHIFT ) ); 
	}

	size_t GetMapFaceIndex( void ) const 
	{ 
		return metadata & DRAWFACE_METADATA_MAP_FACE_INDEX_MASK; 
	}
	
	size_t GetShaderListIndex( void ) const 
	{ 
		return VALUE_FROM_BITS( sort, DRAWFACE_SORT_SHADER_INDEX_MASK, DRAWFACE_SORT_SHADER_INDEX_SHIFT ); 
	}
	
	size_t GetDepthValue( void ) const 
	{ 
		return VALUE_FROM_BITS( sort, DRAWFACE_SORT_DEPTH_MASK, DRAWFACE_SORT_DEPTH_SHIFT ); 
	}
 
	void SetTransparent( bool isTransparent ) 
	{ 
		metadata &= DRAWFACE_METADATA_MAP_FACE_INDEX_MASK;

		// paranoia, because the bool can be anything (at least when compiled to asm.js)
		metadata |= !!( ( int ) isTransparent ) << DRAWFACE_METADATA_IS_TRANSPARENT_SHIFT; 
	}

	void SetMapFaceIndex( size_t index )
	{
		metadata &= ~DRAWFACE_METADATA_MAP_FACE_INDEX_MASK;
		metadata |= index;
	}

	void SetDepthValue( size_t depth )
	{
		SET_BITS_FOR_VALUE( depth, sort, DRAWFACE_SORT_DEPTH_MASK, DRAWFACE_SORT_DEPTH_SHIFT );
	}

	void SetShaderListIndex( size_t shaderIndex )
	{
		SET_BITS_FOR_VALUE( shaderIndex, sort, DRAWFACE_SORT_SHADER_INDEX_MASK, DRAWFACE_SORT_SHADER_INDEX_SHIFT );
	}
};

struct drawPass_t
{
	bool isSolid;
	bool envmap: 1;
	
	int faceIndex, viewLeafIndex;

	passType_t type;
	passDrawType_t drawType;

	uint32_t renderFlags;

	const bspFace_t* face;
	const bspBrush_t* farBrush;
	const bspLeaf_t* leaf;
	const bspLightvol_t* lightvol;

	const shaderInfo_t* shader;

	const viewParams_t& view;

	float minFaceViewDepth, maxFaceViewDepth; 						// max < 0, min > 0 due to RHS and Quake's standards

	const shaderInfo_t* skyShader;
	std::vector< const bspFace_t* > skyFaces;

	bool isDebugDraw;												// set to true before calling DrawFaceList to take the debug path
	std::function< void( drawPass_t& pass ) > debugDrawCallback;	// should follow the same methodology as the callbacks initialized in "DrawFace"
	Program* debugProgram;											// is nullptr by default, so this is user specified.

	std::vector< bool > opaqueFacesVisited, transparentFacesVisited;
	
	std::vector< drawFace_t > transparentFaces, opaqueFaces;

	drawPass_t( const Q3BspMap& map, const viewParams_t& viewData );
};

struct effect_t;
struct shaderStage_t;
struct mapModel_t;

using effectFnSig_t = void( const Program& p, const effect_t& e );

using programMap_t = std::unordered_map<
	std::string,
	std::unique_ptr< Program >
>;

using effectMap_t = std::unordered_map<
	std::string,
	std::function< effectFnSig_t >
>;

using modelBuffer_t = std::vector< std::unique_ptr< mapModel_t > >;

struct debugFace_t
{
	std::vector< glm::vec3 > positions;
	glm::vec4 color;
};

namespace gla {
	struct atlas_t;
}

using gla_atlas_ptr_t = std::unique_ptr< gla::atlas_t >;
using gla_array_t = std::array< gla_atlas_ptr_t, 3 >;

// An instance of this gets passed from Q3BspMap to the BSPRenderer once
// all of the map data has been read.
struct renderPayload_t
{
	gla_array_t textureData;
};

// indices for the above payload's atlas array
enum {
	TEXTURE_ATLAS_SHADERS = 0x0,
	TEXTURE_ATLAS_MAIN = 0x1,
	TEXTURE_ATLAS_LIGHTMAPS = 0x2
};

class RenderBase
{
public:
	programMap_t 					glPrograms;

	std::array< GLuint, 2 >			apiHandles;

	std::unique_ptr< InputCamera > 	camera;

	std::unique_ptr< Frustum > 		frustum;

	std::unique_ptr< ImmDebugDraw > debugRender;

	float							targetFPS;

	float               			deltaTime;

	void MakeProg(
		const std::string& name,
		const std::string& vertPath,
		const std::string& fragPath,
		const std::vector< std::string >& uniforms,
		const std::vector< std::string >& attribs
	);

	virtual void Update( float dt );

	virtual std::string GetBinLayoutString( void ) const;

	RenderBase( float viewWidth, float viewHeight );

	virtual ~RenderBase( void );
};

class BSPRenderer : public RenderBase
{
public:

	// [0] void* -> surface/face data to render
	// [1] const shaderInfo_t* -> relevant shader information
	// [2] int32_t -> texture index
	// [3] int32_t -> lightmap index
	// [4] bool -> true if solid, false if not
	using drawTuple_t = std::tuple<
		const void*,
		const shaderInfo_t*,
		int32_t,
		int32_t,
		bool
	>;

	gla_array_t 					textures;

	// has one->one mapping with face indices
	modelBuffer_t					glFaces;

	// has one-one mapping with
	// face indices - is only used when debugging for immediate data
	std::vector< debugFace_t > 		glDebugFaces;

	effectMap_t						glEffects;

	const bspLeaf_t*    			currLeaf;

	double							frameTime;

	bool							alwaysWriteDepth;

	bool							allowFaceCulling;

	// true -> 
	bool 							skyLinearFilter;	

	viewMode_t						curView;

	Q3BspMap& 						map;

	// -------------------------------
	// Rendering
	// -------------------------------

	void				DrawMapPass(
							int32_t textureIndex,
							int32_t	lightmapIndex,
							std::function< void( const Program& )> callback
						);

	void				DrawEffectPass(
							const drawTuple_t& data,
							drawCall_t callback
						);

	void 				DrawSkyPass( void );

	void				DrawFaceVerts(
							const drawPass_t& pass,
							const shaderStage_t* stage
						) const;
 
	// -------------------------------
	// State management
	// -------------------------------

	void 				BindTexture(
							const Program& program,
							const gla_atlas_ptr_t& atlas,
							uint16_t image,
							const char* prefix,
							int offset
						);

	void				LoadLightVol(
							const drawPass_t& pass,
							const Program& prog
						) const;

	void				DeformVertexes(
							const mapModel_t& m,
							const shaderInfo_t* shader
						) const;

	void				LoadPassParams(
							drawPass_t& pass,
							int32_t face,
							passDrawType_t defaultPass
						) const;

	// -------------------------------
	// BSP Traversal
	// -------------------------------

	void				RenderPass( const viewParams_t& view );

	void				ProcessFace( drawPass_t& pass, uint32_t index );

	void 				DrawModel( drawPass_t& pass, bool frustumCull, bspModel_t& model );

	void				DrawNode( drawPass_t& pass, int32_t nodeIndex );

	void				TraverseDraw( drawPass_t& pass, bool solid );

	void				DrawFaceList(
							drawPass_t& p,
							bool solid
						);

	void				DrawFace( drawPass_t& pass );

	// -------------------------------
	// Init
	// -------------------------------

	void				Prep( void );

	void				Load( renderPayload_t& payload );

	void				LoadVertexData( void );

	// -------------------------------
	// Frame
	// -------------------------------

	void				Render( void );

	float				CalcFPS( void ) const { return 1.0f / ( float )frameTime; }

	// -------------------------------
	// Meta
	// -------------------------------

	virtual std::string GetBinLayoutString( void ) const override;

	uint32_t			GetPassLayoutFlags( passType_t type );


						BSPRenderer( 
							float viewWidth, 
							float viewHeight,
							Q3BspMap& map 
						);

						~BSPRenderer( void );


};
