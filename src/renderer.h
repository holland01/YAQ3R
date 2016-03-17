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

using drawCall_t = std::function< void( const void* param, const Program& program, const shaderStage_t* stage ) >;

struct drawSurface_t
{
	// Every face within a given surface must
	// have the same following 4 values

	int32_t					textureIndex;
	int32_t					lightmapIndex;
	int32_t					faceType;
	const shaderInfo_t*		shader;

	guBufferOffsetList_t			bufferOffsets;
	guBufferRangeList_t				bufferRanges;

	std::vector< int32_t >			drawFaceIndices;

	std::vector< int32_t >			faceIndices; // for vertex deformations

			drawSurface_t( void )
				:	textureIndex( 0 ),
					lightmapIndex( 0 ),
					faceType( 0 ),
					shader( nullptr )
			{}
};

using surfKeyTier0_t = uint8_t; // type
using surfKeyTier1_t = int8_t; // lightmap index
using surfKeyTier2_t = int8_t; // shader index
using surfKeyTier3_t = uintptr_t; // shaderInfo_t* address

using surfMapTier3_t = std::unordered_map< surfKeyTier3_t, drawSurface_t >;
using surfMapTier2_t = std::unordered_map< surfKeyTier2_t, surfMapTier3_t >;
using surfMapTier1_t = std::unordered_map< surfKeyTier1_t, surfMapTier2_t >;

using surfaceContainer_t = std::array< surfMapTier1_t, 4 >;

struct drawSurfaceList_t
{
	surfaceContainer_t surfaces, effectSurfaces;
};

struct drawPass_t
{
	bool isSolid: 1;
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

	std::vector< uint8_t > facesVisited;
	std::vector< int32_t > transparent, opaque;

	drawSurfaceList_t patches;
	drawSurfaceList_t polymeshes;

	drawPass_t( const Q3BspMap* const & map, const viewParams_t& viewData );
};

struct effect_t;
struct shaderStage_t;
struct mapModel_t;

using effectFnSig_t = void( const Program& p, const effect_t& e );
using programMap_t = std::unordered_map< std::string, std::unique_ptr< Program > >;
using effectMap_t = std::unordered_map< std::string, std::function< effectFnSig_t > >;
using modelBuffer_t = std::vector< std::unique_ptr< mapModel_t > >;

struct debugFace_t
{
	std::vector< glm::vec3 > positions;
	glm::vec4 color;
};

class BSPRenderer
{
private:

	// last two integers are textureIndex and lightmapIndex, respectively. the const void* is an optional parameter
	using drawTuple_t = std::tuple< const void*, const shaderInfo_t*, int32_t, int32_t >;

	gSamplerHandle_t				mainSampler; // also used by lightmaps

	gTextureHandle_t				shaderTexHandle, mainTexHandle, lightmapHandle;

	modelBuffer_t					glFaces;			// has one->one mapping with face indices

	std::vector< debugFace_t > glDebugFaces; // has one-one mapping with face indices - is only used when debugging for immediate data

	programMap_t		glPrograms;

	effectMap_t			glEffects;

	const bspLeaf_t*    currLeaf;

	std::array< GLuint, 2 >	apiHandles;

	float               deltaTime;

	double				frameTime;

	void				LoadLightVol( const drawPass_t& pass, const Program& prog ) const;

	void                SortDrawSurfaces( std::vector< drawSurface_t >& surf, bool transparent );

	void				DeformVertexes( const mapModel_t& m, const shaderInfo_t* shader ) const;

	void				MakeProg( const std::string& name, const std::string& vertPath, const std::string& fragPath,
							const std::vector< std::string >& uniforms, const std::vector< std::string >& attribs );

	uint32_t			GetPassLayoutFlags( passType_t type );

	bool				IsTransFace( int32_t faceIndex, const shaderInfo_t* shader ) const;

	void				LoadPassParams( drawPass_t& pass, int32_t face, passDrawType_t defaultPass ) const;

	void				DrawMapPass( int32_t textureIndex, int32_t lightmapIndex, std::function< void( const Program& )> callback );

	void				MakeAddSurface(const shaderInfo_t* shader, int32_t faceIndex, surfaceContainer_t& surfList );

	void				AddSurface( const shaderInfo_t* shader, int32_t faceIndex, surfaceContainer_t& surfList );

	void				ReflectFromTuple( const drawTuple_t& data, const drawPass_t& pass, const Program& program );

	void				DrawSurface( const drawSurface_t& surface ) const;

	void				DrawFaceList( drawPass_t& p, const std::vector< int32_t >& list );

	void				DrawSurfaceList( const surfaceContainer_t& list, bool solid );

	void				DrawEffectPass( const drawTuple_t& data, drawCall_t callback );

	void				ProcessFace( drawPass_t& pass, uint32_t index );

	void				DrawDebugFace( uint32_t index );

	void				DrawNode( drawPass_t& pass, int32_t nodeIndex );

	void				DrawList( drawSurfaceList_t& list, bool solid );

	void				DrawClear( drawPass_t& pass, bool solid );

	void				TraverseDraw( drawPass_t& pass, bool solid );

	void				DrawFace( drawPass_t& pass );

	void				DrawFaceVerts( const drawPass_t& pass, const shaderStage_t* stage ) const;

	void				LoadVertexData( void );

	void				LoadLightmaps( void );

public:
	Q3BspMap*       map;
	InputCamera*	camera;
	Frustum*		frustum;

	viewMode_t		curView;

				BSPRenderer( float viewWidth, float viewHeight );

				~BSPRenderer( void );

	void		Prep( void );

	void		Load( const std::string& filepath );

	void		Sample( void );

	void		Render( void );

	void		RenderPass( const viewParams_t& view );

	float		CalcFPS( void ) const { return 1.0f / ( float )frameTime; }

	void		Update( float dt );
};

INLINE void BSPRenderer::DrawFaceList( drawPass_t& p, const std::vector< int32_t >& list )
{
	passDrawType_t defaultPass = p.drawType;

	for ( int32_t face: list )
	{
		LoadPassParams( p, face, defaultPass );
		DrawFace( p );
	}
}

