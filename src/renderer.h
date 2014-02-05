#pragma once

#include "common.h"
#include "q3m.h"

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

class Input;

class RenderPass
{
public:
    friend class Input;

    RenderPass( void );
    RenderPass( const RenderPass& copy );
    ~RenderPass( void );

    RenderPass& operator =( RenderPass copy );

    void Reset( void );

    const glm::mat4& View( void ) const { return view; }
    const glm::mat4& Projection( void ) const { return projection; }

    glm::mat4       Orientation( void );

    glm::vec3       position;
    glm::vec3       rotation;

private:

    glm::mat4 view, projection;
};


/*
=====================================================

                    BSPRenderer

=====================================================
*/

class BSPRenderer
{
public:

    BSPRenderer( void );

    BSPRenderer( const BSPRenderer& copy );

    ~BSPRenderer( void );

    void    Prep( void );
    void    Load( const std::string& filepath );

    void    Draw( const RenderPass& pass );
    void    Update( float dt, const RenderPass& pass );

private:

    void                DrawTree( int index, const RenderPass& pass );
    void                DrawLeafNode( int index, const RenderPass& pass );

    GLuint              bspProgram;
    GLuint              vao;
    GLuint              vbo;

    Quake3Map           map;

    glm::vec3           lastCameraPosition;

    std::vector< int >  visibleFaces;

    float               deltaTime;

};
