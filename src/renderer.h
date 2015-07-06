#pragma once

#include "common.h"
#include "q3bsp.h"
#include "input.h"
#include "frustum.h"
#include "aabb.h"
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
	std::vector< bspVertex_t >			vertices;
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

    std::vector< byte > facesVisited;
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

class BSPRenderer
{
private:
	friend struct transformStash_t< BSPRenderer >;

	using effectFnSig_t = void( const Program& p, const effect_t& e );

	// last two integers are textureIndex and lightmapIndex, respectively
	// the const void* is either a const drawSurface_t* or const bspFace_t*, depending on objectType_t
	using drawTuple_t	= std::tuple< objectType_t, const void*, const shaderInfo_t*, int, int32_t >; 

	std::unique_ptr< TextureBuffer > glTextureArray, glLightmapArray;

	texture_t					glDummyTexture;
	
	std::vector< texture_t >	glTextures;			// has one->one mapping with texture indices
	
	std::vector< texture_t >	glLightmaps;		// has one->one mapping with lightmap indices
	
	std::vector< mapModel_t >	glFaces;			// has one->one mapping with face indices

	std::map< std::string, std::unique_ptr< Program > >		glPrograms;
	
	std::map< std::string, std::function< effectFnSig_t > >	glEffects;

	const bspLeaf_t*    currLeaf;

    GLuint              vao, vbo;

    float               deltaTime;
	double				frameTime;

	void				LoadLightVol( const drawPass_t& pass, const Program& prog ) const;

	const texture_t&	GetTextureOrDummy( int32_t index, bool predicate, const std::vector< texture_t >& textures ) const;

	void				DeformVertexes( const mapModel_t& m, const shaderInfo_t* shader ) const;

	void				MakeProg( const std::string& name, const std::string& vertPath, const std::string& fragPath,
							const std::vector< std::string >& uniforms, const std::vector< std::string >& attribs, bool bindTransformsUbo );

	void				LoadTransforms( const glm::mat4& view, const glm::mat4& projection ) const;

	uint32_t			GetPassLayoutFlags( passType_t type );

	bool				IsTransFace( int32_t faceIndex, const shaderInfo_t* shader ) const;

	void				LoadPassParams( drawPass_t& pass, int32_t face, passDrawType_t defaultPass ) const;

	void				DrawMapPass( drawPass_t& parms );
	
	void				BeginMapPass( drawPass_t& pass, const texture_t** tex0, const texture_t** tex1 );
	
	void				EndMapPass( drawPass_t& pass, const texture_t* tex0, const texture_t* tex1 );

	void				AddSurface( const shaderInfo_t* shader, int32_t faceIndex, std::vector< drawSurface_t >& surfList );

	void				DrawFromTuple( const drawTuple_t& data, const drawPass_t& pass, const Program& program );

	void				DrawSurface( const drawSurface_t& surface, const Program& program ) const;

	void				DrawFaceList( drawPass_t& p, const std::vector< int32_t >& list );

	void				DrawSurfaceList( const drawPass_t& pass, const std::vector< drawSurface_t >& list );

	void				DrawEffectPass( const drawPass_t& pass, const drawTuple_t& data );

	void				DrawNode( drawPass_t& pass, int32_t nodeIndex );

    void				DrawFace( drawPass_t& pass );

	void				DrawFaceVerts( const drawPass_t& pass, const Program& program ) const;

	void				DrawFaceBounds( const viewParams_t& view, int32_t faceIndex ) const;

public:
	Q3BspMap*       map;
    InputCamera*	camera;
    Frustum*		frustum;

	GLuint			transformBlockIndex;
	GLuint			transformBlockObj;
	size_t			transformBlockSize;

	viewMode_t		curView;

	lightSampler_t	lightSampler;	

				BSPRenderer( void );
				
				~BSPRenderer( void );

    void		Prep( void );
    
	void		Load( const std::string& filepath, uint32_t loadFlags );

	void		Sample( void );
    
	void		Render( void );
	
	void		RenderPass( const viewParams_t& view, bool envmap );

	float		CalcFPS( void ) const { return 1.0f / ( float )frameTime; }

    void		Update( float dt );

	InputCamera* CameraFromView( void );
};

INLINE const texture_t& BSPRenderer::GetTextureOrDummy( int32_t index, 
	bool predicate, const std::vector< texture_t >& textures ) const
{
	if ( predicate )
	{
		return textures[ index ];
	}
	else
	{
		return glDummyTexture;
	}
}

INLINE void BSPRenderer::DrawFaceList( drawPass_t& p, const std::vector< int32_t >& list )
{
	passDrawType_t defaultPass = p.drawType;

	for ( int32_t face: list )
	{
		LoadPassParams( p, face, defaultPass );
		DrawFace( p );
	}
}

INLINE void BSPRenderer::DrawSurface( const drawSurface_t& surf, const Program& program ) const
{
	for ( int32_t i: surf.faceIndices )
	{
		DeformVertexes( glFaces[ i ], surf.shader );
	}

	program.LoadAttribLayout();

	GLenum mode = ( surf.faceType == BSP_FACE_TYPE_PATCH )? GL_TRIANGLE_STRIP: GL_TRIANGLES;

	GL_CHECK( glMultiDrawElements( mode, &surf.indexBufferSizes[ 0 ], 
		GL_UNSIGNED_INT, ( const GLvoid* const * ) &surf.indexBuffers[ 0 ], surf.indexBuffers.size() ) );

#ifdef _DEBUG_FACE_TYPES
	GLint srcFactor, dstFactor;
	GL_CHECK( glGetIntegerv( GL_BLEND_SRC_RGB, &srcFactor ) );
	GL_CHECK( glGetIntegerv( GL_BLEND_DST_RGB, &dstFactor ) );

	GL_CHECK( glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ) );

	glPrograms.at( "debug" )->Bind();

	glm::vec4 color;

	if ( surf.faceType == BSP_FACE_TYPE_PATCH )
	{
		color = glm::vec4( 1.0f, 0.0f, 0.0f, 0.3f );
	}
	else if ( surf.faceType == BSP_FACE_TYPE_POLYGON )
	{
		color = glm::vec4( 0.0f, 1.0f, 0.0f, 0.3f );
	}
	else
	{
		color = glm::vec4( 0.0f, 0.0f, 1.0f, 0.3f );
	}

	glPrograms.at( "debug" )->LoadVec4( "fragColor", color );

	GL_CHECK( glMultiDrawElements( mode, &surf.indexBufferSizes[ 0 ], 
		GL_UNSIGNED_INT, ( const GLvoid* const * ) &surf.indexBuffers[ 0 ], surf.indexBuffers.size() ) );

	glPrograms.at( "debug" )->Release();

	GL_CHECK( glBlendFunc( srcFactor, dstFactor ) );
#endif
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

INLINE InputCamera* BSPRenderer::CameraFromView( void )
{
	InputCamera* pCamera = nullptr;

	switch ( curView )
	{
		case VIEW_MAIN:
			pCamera = camera;
			break;
		case VIEW_LIGHT_SAMPLE:
			pCamera = &lightSampler.camera;
			break;
	}

	return pCamera;
}