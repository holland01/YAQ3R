#pragma once

#include "common.h"
/*
=====================================================

                    GLCamera

=====================================================
*/

class GLCamera
{
public:
    GLCamera( void );

    void evalKeyPress( int key );

    void evalMouseCoords( float x, float y );

    void walk( float step );
    void strafe( float step );
    void raise ( float step );

    void updateView( void );

    void setPerspective( float fovy, float aspect, float zNear, float zFar );

    void reset( void );

    const glm::mat4& view( void ) const { return mView; }
    const glm::mat4& projection( void ) const { return mProjection; }

    glm::mat4 orientation( void );

    glm::vec3   mPosition;

    glm::vec3   mRotation;

private:

    glm::mat4   mView, mProjection;

    glm::vec2   mLastMouse;
};


/*
=====================================================

                    GLRenderer

=====================================================
*/

class Quake3Map;

class BSPRenderer
{
public:

    GLCamera   mCamera;

    BSPRenderer( void );

    ~BSPRenderer( void );

    void allocBase( void );

    void loadMap( const std::string& filepath );

    void draw( void );

    void update( void );

private:

    GLuint              mVao, mBuffers[ 2 ];

    Quake3Map*          mMap;

    glm::vec3           mLastCameraPosition;

    byte*               mVisibleFaces;

    GLuint              mBspProgram;

};
