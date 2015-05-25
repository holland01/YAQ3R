#pragma once

#include "common.h"
#include "q3bsp.h"
#include "input.h"
#include "frustum.h"
#include "aabb.h"

/*
=====================================================

Author: Holland Schutte
License: WTFPL

                   renderer.h

    Contains the driver functionality for the Quake3Map class,
    along with a camera class to perform basic view transformations.
    Provides the necessary functionality to get the map on the screen.

=====================================================
*/

/*
=====================================================

                    RenderPass

=====================================================
*/

class RenderPass
{
public:
    RenderPass( const Q3BspMap* const & map, const viewParams_t& viewData );
    ~RenderPass( void );

	const bspLeaf_t*		leaf;
    const viewParams_t&     view;

    std::vector< byte >     facesRendered;
};

/*
=====================================================

                    BSPRenderer

=====================================================
*/

// Draw flags
enum 
{
	RENDER_BSP_LIGHTMAP_INFO = 1 << 0,
	RENDER_BSP_EFFECT = 1 << 1,
	RENDER_BSP_ALWAYS_POLYGON_OFFSET = 1 << 2,
	RENDER_BSP_USE_TCMOD = 1 << 3
};

struct drawFace_t
{
	bool isSolid: 1;

	int faceIndex;

	uint32_t renderFlags;

	RenderPass* pass;
	AABB* bounds;
	
	const bspBrush_t* brush;
	const bspFace_t* face;
	const shaderInfo_t* shader;
};

class BSPRenderer
{
public:

    InputCamera*	camera;
    Frustum*		frustum;

	int				mapDimsLength;
	int				lodThreshold;

	GLuint			transformBlockIndex;
	GLuint			transformBlockObj;
	size_t			transformBlockSize;

    BSPRenderer( void );
    ~BSPRenderer( void );

    void    Prep( void );
    void    Load( const std::string& filepath, uint32_t loadFlags );

    void    Render( uint32_t renderFlags );

    void    DrawNode( int nodeIndex, RenderPass& pass, bool isSolid, uint32_t renderFlags );
	void	DrawFaceNoEffect( drawFace_t* parms );
    void    DrawFace( drawFace_t* parms );
	void	DrawFaceVerts( drawFace_t* parms );

	float   CalcFPS( void ) const { return 1.0f / ( float )frameTime; }

    void    Update( float dt );

private:

    Q3BspMap*           map;
	const bspLeaf_t*    currLeaf;

    GLuint              bspProgram;
    GLuint              vao, vbo;

    float               deltaTime;
	double				frameTime;

	std::map< std::string, GLint > bspProgramUniforms;

	void DeformVertexes( mapModel_t* m, drawFace_t* parms );
	int CalcSubdivision( const RenderPass& pass, const AABB& bounds );
};
