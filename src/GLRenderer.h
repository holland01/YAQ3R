#pragma once

#include "common.h"
#include <qmatrix.h>

/*
===========================

        GLCamera

===========================
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

    QMatrix4x4 orientation( void );

private:

    QVector3D   mRotation, mPosition;

    QMatrix4x4  mView, mProjection;
};

