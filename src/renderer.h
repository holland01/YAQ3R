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
	PASS_MODEL
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

enum viewMode_t
{
	VIEW_MAIN = 0,
	VIEW_LIGHT_SAMPLE,
};

class BSPRenderer
{
private:

	struct {
		glm::mat4 view, projection;
		GLuint fbo;
		texture_t attachment;
	} lightSampler;

    Q3BspMap*           map;
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

public:

    InputCamera*	camera;
    Frustum*		frustum;

	int				mapDimsLength;
	int				lodThreshold;

	GLuint			transformBlockIndex;
	GLuint			transformBlockObj;
	size_t			transformBlockSize;

	viewMode_t		curView;

    BSPRenderer( void );
    ~BSPRenderer( void );

    void    Prep( void );
    void    Load( const std::string& filepath, uint32_t loadFlags );

    void    Render( uint32_t renderFlags );

    void    DrawNode( int nodeIndex, drawPass_t& pass );
	void	DrawMapPass( drawPass_t& parms );
    void    DrawFace( drawPass_t& parms );
	void	DrawFaceVerts( drawPass_t& parms, bool isEffectPass );

	bool	CalcLightVol( const glm::vec3& position );

	float   CalcFPS( void ) const { return 1.0f / ( float )frameTime; }

    void    Update( float dt );


};

INLINE void BSPRenderer::DrawFaceList( drawPass_t& p, const std::vector< int >& list )
{
	for ( int face: list )
	{
		p.face = &map->data.faces[ face ]; 
		p.faceIndex = face;
		p.shader = map->GetShaderInfo( face );
		DrawFace( p );
	}
}