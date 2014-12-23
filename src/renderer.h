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
    void    Load( const std::string& filepath );

    void    DrawWorld( void );

    void    DrawNode( int nodeIndex, RenderPass& pass, bool isSolid );
    void    DrawFace( int faceIndex, RenderPass& pass, const AABB& bounds, bool isSolid );

    void    Update( float dt );

private:

    typedef bool        ( *PredicateFunc )( const bspVertex_t&, const bspVertex_t& );

    Q3BspMap*           map;

    GLuint              bspProgram;
    GLuint              vao, vbo;

    float               deltaTime;

    const bspLeaf_t*    currLeaf;

	int CalcSubdivision( const RenderPass& pass, const AABB& bounds );
};
