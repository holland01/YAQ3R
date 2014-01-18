#pragma once

#include "common.h"
#include <qmatrix.h>

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

    void rotateX( float angDeg );
    void rotateY( float angDeg );

    void updateView( void );

    void setPerspective( float fovy, float aspect, float zNear, float zFar );

    void reset( void );

    const QMatrix4x4& view( void ) const { return mView; }
    const QMatrix4x4& projection( void ) const { return mProjection; }

    const QVector3D& position( void ) const { return mPosition; }

    QMatrix4x4 orientation( void );

    QVector3D   mPosition;

    QVector3D   mRotation;

private:

    QMatrix4x4  mView, mProjection;
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

    void render( void );

    void update( void );

private:

    GLuint              mVao;

    //                  Matrix uniform locations
    int                 mProjectionUnif, mModelViewUnif;

    QGLBuffer           mIndexBuffer;
    QGLBuffer           mVertexBuffer;

    QGLShaderProgram    mProgram;

    Quake3Map*          mMapData;

    //std::set< long unsigned int > mAlreadyVisible; // faces already visible

    std::set< int >     mVisibleFaces;
    std::vector< int >  mIndicesBuffer;
    std::vector< QVector3D > mVerticesBuffer;

    QVector3D           mLastCameraPosition;

    bool linkProgram( const QString& vertexShaderPath, const QString& fragmentShaderPath );
};
