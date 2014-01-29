#pragma once

#include "common.h"

#define BSP_RENDERER_NUM_VBOS 2

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

    void EvalMouseMove( float x, float y );

    void Walk( float step );
    void Strafe( float step );
    void Raise( float step );

    void UpdateView( void );

    void SetPerspective( float fovy, float aspect, float zNear, float zFar );

    void Reset( void );

    const glm::mat4& View( void ) const { return view; }
    const glm::mat4& Projection( void ) const { return projection; }

    glm::mat4   orientation( void );

    glm::vec3   position;

    glm::vec3   rotation;

private:

    glm::mat4   view, projection;

    glm::vec2   lastMouse;
};


/*
=====================================================

                    BSPRenderer

=====================================================
*/

class Quake3Map;

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

    GLuint              vao, bufferObjects[ BSP_RENDERER_NUM_VBOS ];

    Quake3Map*          map;

    glm::vec3           lastCameraPosition;

    byte*               visibleFaces;

    GLuint              bspProgram;

};
