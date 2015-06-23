#pragma once

#include "common.h"
#include "q3bsp.h"
#include "input.h"
#include "frustum.h"
#include "aabb.h"
#include <set>
#include <array>
#include <functional>

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
	PASS_EFFECT = 0,
	PASS_MAP,
	PASS_MODEL,
	PASS_LIGHT_SAMPLE
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
	bool						deform;
	GLuint						vbo;
	int32_t						subdivLevel;

	// used if face type == mesh or polygon
	std::vector< int32_t >				indices;

	// used if face type == patch  
	std::vector< const bspVertex_t* >	controlPoints; // control point elems are stored in multiples of 9
	std::vector< bspVertex_t >			vertices;
	std::vector< int32_t* >				rowIndices;
	std::vector< int32_t  >				trisPerRow;

	mapModel_t( void );
	~mapModel_t( void );
};

struct drawSurface_t
{
	// Every face within a given surface must
	// have the same following three values
	const shaderInfo_t*			shader;
	int							textureIndex;
	int							lightmapIndex;

	std::vector< int32_t*	>	indexBuffers;
	std::vector< int32_t	>	indexBufferSizes;

			drawSurface_t( void ): shader( nullptr )
			{}
};

struct drawPass_t
{
	bool isSolid: 1;

	int faceIndex, viewLeafIndex;

	passType_t type;
	uint32_t renderFlags;

	AABB* bounds;

	const Program* program;

	const bspBrush_t* brush;
	const bspFace_t* face;
	const bspLeaf_t* leaf;
	const bspLightvol_t* lightvol;
	const shaderInfo_t* shader;

	const viewParams_t& view;

    std::vector< byte > facesVisited;
	std::vector< int > transparent, opaque;
	std::vector< drawSurface_t > transSurfaces, opaqueSurfaces;

	drawPass_t( const Q3BspMap* const & map, const viewParams_t& viewData );
	~drawPass_t( void );
};

struct lightSampler_t {
	static const int NUM_BUFFERS = 2;
	
	InputCamera								camera;
	glm::vec4								targetPlane;
	glm::vec2								boundsMin, boundsMax;

	std::array< GLuint, NUM_BUFFERS >		fbos;
	std::array< texture_t, NUM_BUFFERS >	attachments;

	lightSampler_t( void );
	
	~lightSampler_t( void );

	void				Bind( int which ) const;
	
	void				Release( void ) const;
	
	void				Elevate( const glm::vec3& min, const glm::vec3& max );
};

struct effect_t;

class BSPRenderer
{
private:
	using effectFnSig_t = void( const Program& p, const effect_t& e );

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

	const texture_t&	GetTextureOrDummy( int index, bool predicate, const std::vector< texture_t >& textures ) const;

	void				DeformVertexes( mapModel_t* m, drawPass_t& parms );

	void				MakeProg( const std::string& name, const std::string& vertPath, const std::string& fragPath,
							const std::vector< std::string >& uniforms, const std::vector< std::string >& attribs, bool bindTransformsUbo );

	void				LoadTransforms( const glm::mat4& view, const glm::mat4& projection );

	uint32_t			GetPassLayoutFlags( passType_t type );

	bool				IsTransFace( int faceIndex, const shaderInfo_t* shader ) const;

	void				LoadPassParams( drawPass_t& pass, int face, passType_t defaultPass ) const;

	void				DrawMapPass( drawPass_t& parms );
	
	void				BeginMapPass( drawPass_t& pass, const texture_t** tex0, const texture_t** tex1 );
	
	void				EndMapPass( drawPass_t& pass, const texture_t* tex0, const texture_t* tex1 );

	void				AddSurfaceOnlyIf( drawPass_t& pass, 
							const shaderInfo_t* shader, bool predicate, int faceIndex, std::vector< drawSurface_t >& surfList );

	void				DrawFaceList( drawPass_t& p, const std::vector< int >& list );

	void				DrawEffectPass( drawPass_t& pass );

	void				DrawSurfaceList( drawPass_t& pass, const std::vector< drawSurface_t >& list );
	
	void				DrawFaceOnlyIf( drawPass_t& pass, 
							bool predicate, int faceIndex, passType_t defaultPass );
	
	void				DrawNode( int nodeIndex, drawPass_t& pass );

    void				DrawFace( drawPass_t& pass );

	void				DrawFaceVerts( drawPass_t& pass );

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

	void		Sample( uint32_t renderFlags );
    void		Render( uint32_t renderFlags );

	float		CalcFPS( void ) const { return 1.0f / ( float )frameTime; }

    void		Update( float dt );

	InputCamera* CameraFromView( void );
};

INLINE const texture_t& BSPRenderer::GetTextureOrDummy( int index, 
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

INLINE void BSPRenderer::LoadPassParams( drawPass_t& p, int face, passType_t defaultPass ) const
{
	p.face = &map->data.faces[ face ];
	p.faceIndex = face;
	p.shader = map->GetShaderInfo( face );

	if ( p.shader )
	{
		p.type = PASS_EFFECT;
	}
	else
	{
		p.type = defaultPass;
	}
}

INLINE void BSPRenderer::DrawFaceOnlyIf( drawPass_t& pass, bool predicate, int faceIndex, passType_t defaultPass )
{
	if ( predicate )
	{
		LoadPassParams( pass, faceIndex, PASS_MAP );
		DrawFace( pass );
		pass.facesVisited[ faceIndex ] = true;
	}
}

INLINE void BSPRenderer::AddSurfaceOnlyIf( drawPass_t& pass, 
		const shaderInfo_t* shader, bool predicate, int faceIndex, std::vector< drawSurface_t >& surfList )
{
	if ( predicate )
	{
		const bspFace_t* face = &map->data.faces[ faceIndex ]; 
		
		bool add = true;
		for ( drawSurface_t& surf: surfList )
		{
			if ( shader == surf.shader && face->texture == surf.textureIndex && face->lightmapIndex == surf.lightmapIndex )
			{
				surf.indexBuffers.push_back( &glFaces[ faceIndex ].indices[ 0 ] );
				surf.indexBufferSizes.push_back( glFaces[ faceIndex ].indices.size() );
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
			
			surfList.push_back( std::move( surf ) ); 
		}

		pass.facesVisited[ faceIndex ] = true;
	}
}

INLINE void BSPRenderer::DrawFaceList( drawPass_t& p, const std::vector< int >& list )
{
	passType_t defaultPass = p.type;

	for ( int face: list )
	{
		LoadPassParams( p, face, defaultPass );
		DrawFace( p );
	}
}

INLINE void BSPRenderer::LoadTransforms( const glm::mat4& view, const glm::mat4& projection )
{
	GL_CHECK( glBindBuffer( GL_UNIFORM_BUFFER, transformBlockObj ) );
	GL_CHECK( glBufferSubData( GL_UNIFORM_BUFFER, 0, sizeof( glm::mat4 ), glm::value_ptr( projection ) ) );

	GL_CHECK( glBufferSubData( GL_UNIFORM_BUFFER, sizeof( glm::mat4 ), sizeof( glm::mat4 ), glm::value_ptr( view ) ) );
	GL_CHECK( glBindBuffer( GL_UNIFORM_BUFFER, 0 ) );
}

INLINE uint32_t BSPRenderer::GetPassLayoutFlags( passType_t type )
{
	switch ( type )
	{
		case PASS_MAP:
			return GLUTIL_LAYOUT_ALL ^ GLUTIL_LAYOUT_NORMAL;
			break;
		case PASS_EFFECT:
			return GLUTIL_LAYOUT_POSITION | GLUTIL_LAYOUT_COLOR | GLUTIL_LAYOUT_TEX0;
			break;
		case PASS_MODEL:
			return GLUTIL_LAYOUT_ALL;
			break;
		case PASS_LIGHT_SAMPLE:
			return GLUTIL_LAYOUT_POSITION | GLUTIL_LAYOUT_NORMAL;
			break;
	}

	// Shouldn't happen...
	return 0;
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