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
	PASS_DRAW_MAIN
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
	DRAWFACE_SORT_SHADER_INDEX_MASK = 0x1FF,
	DRAWFACE_SORT_DEPTH_SHIFT = 10,
	DRAWFACE_SORT_DEPTH_BITS = 21,
	DRAWFACE_SORT_DEPTH_MASK = (1 << DRAWFACE_SORT_DEPTH_BITS) - 1,

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
		return sort & DRAWFACE_SORT_SHADER_INDEX_MASK; 
	}
	
	size_t GetDepthValue( void ) const 
	{ 
		return ( sort >> DRAWFACE_SORT_DEPTH_SHIFT ) & DRAWFACE_SORT_DEPTH_MASK; 
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
		depth &= DRAWFACE_SORT_DEPTH_MASK;
		depth <<= DRAWFACE_SORT_DEPTH_SHIFT;

		sort &= DRAWFACE_SORT_SHADER_INDEX_MASK;
		sort |= depth;
	}

	void SetShaderListIndex( size_t shaderIndex )
	{
		sort &= ~DRAWFACE_SORT_SHADER_INDEX_MASK;
		sort |= shaderIndex & DRAWFACE_SORT_SHADER_INDEX_MASK; 
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

	float minFaceViewDepth, maxFaceViewDepth; // max < 0, min > 0 due to RHS and Quake's standards

	std::vector< uint8_t > facesVisited;
	
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
	programMap_t glPrograms;

	std::array< GLuint, 2 >	apiHandles;

	Q3BspMap& map;

	std::unique_ptr< Frustum > frustum;

	void 			MakeProg(
						const std::string& name,
						const std::string& vertPath,
						const std::string& fragPath,
						const std::vector< std::string >& uniforms,
						const std::vector< std::string >& attribs
					);

public:
	virtual void Load( renderPayload_t& payload );

	virtual std::string GetBinLayoutString( void ) const;

	RenderBase( Q3BspMap& m );

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

	gla_array_t textures;

	// has one->one mapping with face indices
	modelBuffer_t					glFaces;

	// has one-one mapping with
	// face indices - is only used when debugging for immediate data
	std::vector< debugFace_t > glDebugFaces;

	effectMap_t			glEffects;

	const bspLeaf_t*    currLeaf;

	float               deltaTime;

	double				frameTime;

	float				targetFPS;

	bool				alwaysWriteDepth;

	bool				allowFaceCulling;

	std::unique_ptr< InputCamera > camera;

	viewMode_t		curView;

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

	void				Load( renderPayload_t& payload ) override;

	void				LoadVertexData( void );

	// -------------------------------
	// Frame
	// -------------------------------


	void				Render( void );

	void				Update( float dt );

	float				CalcFPS( void ) const { return 1.0f / ( float )frameTime; }

	// -------------------------------
	// Meta
	// -------------------------------

	virtual std::string GetBinLayoutString( void ) const override;

	uint32_t			GetPassLayoutFlags( passType_t type );


						BSPRenderer( float viewWidth, float viewHeight,
						 Q3BspMap& map );

						~BSPRenderer( void );


};
