#pragma once

#include "common.h"
#include "q3bsp.h"
#include "input.h"
#include "frustum.h"
#include "aabb.h"
#include <set>

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

struct drawPass_t
{
	bool isSolid: 1;

	int faceIndex;

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

	drawPass_t( const Q3BspMap* const & map, const viewParams_t& viewData );
	~drawPass_t( void );
};

struct lightSampler_t {
	InputCamera camera;
	GLuint fbo;
	texture_t attachment;
	glm::vec4 targetPlane;

	lightSampler_t( void );
	~lightSampler_t( void );

	void Bind( void ) const;
	void Release( void ) const;
	void Elevate( const glm::vec3& min, const glm::vec3& max );
};

class BSPRenderer
{
private:

	const bspLeaf_t*    currLeaf;

    GLuint              vao, vbo;

    float               deltaTime;
	double				frameTime;

	std::map< std::string, std::unique_ptr< Program > > programs;

	void DeformVertexes( mapModel_t* m, drawPass_t& parms );
	
	void DrawFaceList( drawPass_t& p, const std::vector< int >& list );

	void DrawDebugInfo( drawPass_t& pass );

	void BSPRenderer::DrawEffectPass( drawPass_t& pass );

	void BindTextureOrDummy( bool predicate, int index, int offset, 
		const Program& program, const std::string& samplerUnif, const std::vector< texture_t >& source );

	int CalcSubdivision( const drawPass_t& pass, const AABB& bounds );

	int CalcLightvolIndex( const drawPass_t& pass ) const; 

	void MakeProg( const std::string& name, const std::string& vertPath, const std::string& fragPath,
		const std::vector< std::string >& uniforms, const std::vector< std::string >& attribs, bool bindTransformsUbo );

	void LoadTransforms( const glm::mat4& view, const glm::mat4& projection );

	uint32_t GetPassLayoutFlags( passType_t type );

public:
	Q3BspMap*       map;
    InputCamera*	camera;
    Frustum*		frustum;

	int				mapDimsLength;
	int				lodThreshold;

	GLuint			transformBlockIndex;
	GLuint			transformBlockObj;
	size_t			transformBlockSize;

	viewMode_t		curView;

	lightSampler_t	lightSampler;	

    BSPRenderer( void );
    ~BSPRenderer( void );

    void    Prep( void );
    void    Load( const std::string& filepath, uint32_t loadFlags );

	void	Sample( uint32_t renderFlags );
    void    Render( uint32_t renderFlags );
	void	RenderMain( uint32_t renderFlags );

    void    DrawNode( int nodeIndex, drawPass_t& pass );
	void	DrawMapPass( drawPass_t& parms );
    void    DrawFace( drawPass_t& parms );
	void	DrawFaceVerts( drawPass_t& parms );

	bool	CalcLightVol( const glm::vec3& position );

	float   CalcFPS( void ) const { return 1.0f / ( float )frameTime; }

    void    Update( float dt );

	InputCamera* CameraFromView( void );
};

INLINE void BSPRenderer::DrawFaceList( drawPass_t& p, const std::vector< int >& list )
{
	passType_t defaultPass = p.type;

	for ( int face: list )
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
			return GLUTIL_LAYOUT_ALL & ~GLUTIL_LAYOUT_NORMAL;
			break;
		case PASS_EFFECT:
			return GLUTIL_LAYOUT_POSITION | GLUTIL_LAYOUT_COLOR | GLUTIL_LAYOUT_TEX0;
			break;
		case PASS_MODEL:
			return GLUTIL_LAYOUT_ALL;
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