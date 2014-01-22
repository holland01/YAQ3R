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

    glm::mat4  mView, mProjection;
};


/*
=====================================================

                    GLRenderer

=====================================================
*/

class Quake3Map;

class GLRenderer
{
public:

    GLCamera   mCamera;

    GLRenderer( void );

    ~GLRenderer( void );

    void allocBase( void );

    void loadMap( const std::string& filepath );

    void draw( void );

    void update( void );

private:

    GLuint              mVao;

    Quake3Map*          mMap;

    glm::vec3           mLastCameraPosition;

    byte*               mVisibleFaces;

    GLuint              mBspProgram;

};
