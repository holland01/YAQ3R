#pragma once

#include "common.h"
#include "q3bsp.h"
#include "input.h"
#include "frustum.h"

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

    std::vector< unsigned char >     facesRendered;
};

/*
=====================================================

                    BSPRenderer

=====================================================
*/

class AABB;

// Draw flags
enum 
{
	RENDER_BSP_LIGHTMAP_INFO = 1 << 0,
	RENDER_BSP_EFFECT = 1 << 1
};

class BSPRenderer
{
public:

    InputCamera*	camera;

    Frustum*		frustum;

	BezPatch		patchRenderer;

	int				mapDimsLength;
	int				lodThreshold;

    BSPRenderer( void );
    ~BSPRenderer( void );

    void    Prep( void );
    void    Load( const std::string& filepath, uint32_t loadFlags );

    void    Render( uint32_t renderFlags );

    void    DrawNode( int nodeIndex, RenderPass& pass, bool isSolid, uint32_t renderFlags );
	void	DrawFaceNoEffect( int faceIndex, RenderPass& pass, const AABB& bounds, bool isSolid );
    void    DrawFace( int faceIndex, RenderPass& pass, const AABB& bounds, bool isSolid, uint32_t renderFlags );
	void	DrawFaceVerts( int faceIndex, int subdivLevel );

	float   CalcFps( void ) const { return 60.0f / ( float )frameTime; }

    void    Update( float dt );

private:

    Q3BspMap*           map;

    GLuint              bspProgram;
    GLuint              vao, vbo;

    float               deltaTime;
	double				frameTime;

    const bspLeaf_t*    currLeaf;

	std::map< std::string, GLint > bspProgramUniforms;

	int CalcSubdivision( const RenderPass& pass, const AABB& bounds );
};
