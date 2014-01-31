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

                    Camera

=====================================================
*/

class Camera
{
public:
    Camera( void );

    void EvalKeyPress( int key );
    void EvalKeyRelease( int key );
    void EvalMouseMove( float x, float y );

    void UpdateView( void );

    void SetPerspective( float fovy, float aspect, float zNear, float zFar );

    void Reset( void );

    const glm::mat4& View( void ) const { return view; }
    const glm::mat4& Projection( void ) const { return projection; }

    glm::mat4   Orientation( void );

    glm::vec3   position;
    glm::vec3   rotation;

    glm::vec2   mouseBoundries;

private:

    glm::mat4   view, projection;

    int         keysPressed[ 6 ];
};


/*
=====================================================

                    BSPRenderer

=====================================================
*/

class BSPRenderer
{
public:

    Camera   camera;

    BSPRenderer( void );

    ~BSPRenderer( void );

    void Prep( void );

    void Load( const std::string& filepath );

    void Draw( void );

    void Update( void );

private:

    GLuint              bspProgram;
    GLuint              vao;
    GLuint              vbo;

    Quake3Map           map;

    glm::vec3           lastCameraPosition;

    byte*               visibleFaces;

};
