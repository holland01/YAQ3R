#pragma once

#include "common.h"
#include "q3bsp.h"
#include "input.h"
#include "frustum.h"
#include "aabb.h"
#include "glutil.h"
#include "renderer/texture.h"
#include <set>
#include <array>
#include <functional>
#include <cfloat>

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

struct drawIndirect_t
{
	uint32_t count;
    uint32_t instanceCount;
    uint32_t firstIndex;
    uint32_t baseVertex;
    uint32_t baseInstance;
};

struct mapModel_t
{
	bool						deform: 1;
	GLuint						vboOffset;
	int32_t						subdivLevel;
	std::shared_ptr< rtt_t >	envmap;

	// used if face type == mesh or polygon
	std::vector< int32_t >				indices;

	// used if face type == patch  
	std::vector< const bspVertex_t* >	controlPoints; // control point elems are stored in multiples of 9
    std::vector< bspVertex_t >			patchVertices;
	std::vector< int32_t* >				rowIndices;
	std::vector< int32_t  >				trisPerRow;
	
	AABB								bounds;

	mapModel_t( void );
	~mapModel_t( void );

	void								CalcBounds( int32_t faceType, const mapData_t& data );
};

struct drawSurface_t
{
	// Every face within a given surface must
	// have the same following 4 values
	
	int32_t						textureIndex;
	int32_t						lightmapIndex;
	int32_t						faceType;
	const shaderInfo_t*			shader;
	

	std::vector< const int32_t*		>	indexBuffers;
	std::vector< int32_t			>	indexBufferSizes;
	std::vector< int32_t			>	faceIndices;			

			drawSurface_t( void )
				:	textureIndex( 0 ),
					lightmapIndex( 0 ),
					faceType( 0 ),
					shader( nullptr )
			{}
};

struct drawSurfaceList_t
{
	std::vector< drawSurface_t > surfaces, effectSurfaces;
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

struct lightSampler_t {
	static const int32_t NUM_BUFFERS = 2;
	
	InputCamera								camera;
	glm::vec4								targetPlane;
	glm::vec2								boundsMin, boundsMax;

	std::array< GLuint, NUM_BUFFERS >		fbos;
	std::array< texture_t, NUM_BUFFERS >	attachments;

	lightSampler_t( void );
	
	~lightSampler_t( void );

	void				Bind( int32_t which ) const;
	
	void				Release( void ) const;
	
	void				Elevate( const glm::vec3& min, const glm::vec3& max );
};

struct effect_t;
struct shaderStage_t;

class BSPRenderer
{
private:
	friend struct transformStash_t< BSPRenderer >;

	using effectFnSig_t = void( const Program& p, const effect_t& e );

    // last two integers are textureIndex and lightmapIndex, respectively. the const void* is an optional parameter
    using drawTuple_t = std::tuple< const void*, const shaderInfo_t*, int32_t, int32_t >;

	std::unique_ptr< textureArray_t > glTextureArray, 
									  glLightmapArray,
									  glShaderArray;

    gImageParams_t					glDummyTexture;

	std::array< glm::vec2, GLConfig::MAX_MIP_LEVELS > glDummyBiases;
	
    std::vector< gImageParams_t >	glTextures;			// has one->one mapping with texture indices
	
    std::vector< gImageParams_t >	glLightmaps;		// has one->one mapping with lightmap indices
	
	std::vector< mapModel_t >	glFaces;			// has one->one mapping with face indices

	std::map< std::string, std::unique_ptr< Program > >		glPrograms;
	
	std::map< std::string, std::function< effectFnSig_t > >	glEffects;

	const bspLeaf_t*    currLeaf;

    GLuint              vao, vbo;

    float               deltaTime;

	double				frameTime;

    using               drawCall_t = std::function< void( const void* param, const Program& program, const shaderStage_t* stage ) >;

	void				LoadTextureArray( std::unique_ptr< textureArray_t >& texArray, 
                            std::vector< gImageParams_t >& textures, int32_t width, int32_t height );

	void				LoadLightVol( const drawPass_t& pass, const Program& prog ) const;

	void				DeformVertexes( const mapModel_t& m, const shaderInfo_t* shader ) const;

	void				MakeProg( const std::string& name, const std::string& vertPath, const std::string& fragPath,
							const std::vector< std::string >& uniforms, const std::vector< std::string >& attribs, bool bindTransformsUbo );

	void				LoadTransforms( const glm::mat4& view, const glm::mat4& projection ) const;

	uint32_t			GetPassLayoutFlags( passType_t type );

	bool				IsTransFace( int32_t faceIndex, const shaderInfo_t* shader ) const;

	void				LoadPassParams( drawPass_t& pass, int32_t face, passDrawType_t defaultPass ) const;

    void				DrawMapPass( int32_t textureIndex, int32_t lightmapIndex, std::function< void( const Program& )> callback );

	void				AddSurface( const shaderInfo_t* shader, int32_t faceIndex, std::vector< drawSurface_t >& surfList );

	void				ReflectFromTuple( const drawTuple_t& data, const drawPass_t& pass, const Program& program );

    void				DrawSurface( const drawSurface_t& surface, const shaderStage_t* stage, const Program& program ) const;

	void				DrawFaceList( drawPass_t& p, const std::vector< int32_t >& list );

    void				DrawSurfaceList( const std::vector< drawSurface_t >& list );

    void				DrawEffectPass( const drawTuple_t& data, drawCall_t callback );

	void				DrawNode( drawPass_t& pass, int32_t nodeIndex );

    void				DrawFace( drawPass_t& pass );

    void				DrawFaceVerts( const drawPass_t& pass, const shaderStage_t* stage, const Program& program ) const;

	void				DrawFaceBounds( const viewParams_t& view, int32_t faceIndex ) const;

public:
	Q3BspMap*       map;
    InputCamera*	camera;
    Frustum*		frustum;

	GLuint			transformBlockIndex;
	GLuint			transformBlockObj;
	size_t			transformBlockSize;

	viewMode_t		curView;	

				BSPRenderer( float viewWidth, float viewHeight );
				
				~BSPRenderer( void );

    void		Prep( void );
    
	void		Load( const std::string& filepath, uint32_t loadFlags );

	void		Sample( void );
    
	void		Render( void );
	
    void		RenderPass( const viewParams_t& view, bool envmap );

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

INLINE void BSPRenderer::LoadTransforms( const glm::mat4& view, const glm::mat4& projection ) const
{
	GL_CHECK( glBindBuffer( GL_UNIFORM_BUFFER, transformBlockObj ) );
	GL_CHECK( glBufferSubData( GL_UNIFORM_BUFFER, 0, sizeof( glm::mat4 ), glm::value_ptr( projection ) ) );

	//glm::mat4 viewMod( view );
	//viewMod[ 3 ].z -= 200.0f;

    GL_CHECK( glBufferSubData( GL_UNIFORM_BUFFER, sizeof( glm::mat4 ), sizeof( glm::mat4 ), glm::value_ptr( view ) ) );
	GL_CHECK( glBindBuffer( GL_UNIFORM_BUFFER, 0 ) );
}
