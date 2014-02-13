#pragma once

#include "common.h"
#include "q3m.h"
#include "input.h"

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
    RenderPass( const Quake3Map* const map, const ViewParams& viewData );
    ~RenderPass( void );

    void                    SetFaceCount( int count );

    const ViewParams&       view;
    std::vector< bool >     facesRendered;

};

INLINE void RenderPass::SetFaceCount( int count )
{
    facesRendered.reserve( count );
    facesRendered.assign( count, false );
}


/*
=====================================================

                    BSPRenderer

=====================================================
*/

class BSPRenderer
{
public:

    InputCamera camera;

    BSPRenderer( void );

    ~BSPRenderer( void );

    void    Prep( void );
    void    Load( const std::string& filepath );

    void    DrawWorld( void );

    void    DrawNode( int nodeIndex, RenderPass& pass, bool isSolid );
    void    DrawFace( int faceIndex, RenderPass& pass, bool isSolid );

    void    Update( float dt );

private:

    typedef bool        ( *PredicateFunc )( const BSPVertex&, const BSPVertex& );

    GLuint              bspProgram;
    GLuint              vao, vbo;
    GLuint              texObj;

    float               deltaTime;

    std::vector< int >  alreadyVisible;

    std::vector< int >  visibleFaces;
    std::vector< bool > visibleClusters;

    int                 currClusterIndex;

    Quake3Map           map;
};
