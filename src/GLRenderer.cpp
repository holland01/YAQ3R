#include "GLRenderer.h"


GLCamera::GLCamera( void )
    : mRotation( 0.0f, 0.0f, 0.0f ),
      mPosition( 0.0f, 0.0f, 0.0f )
{
    mView.setToIdentity();
    mProjection.setToIdentity();
}

QMatrix4x4 GLCamera::orientation( void )
{
    QMatrix4x4 orient;
    orient.setToIdentity();

    orient.rotate( mRotation.x(), QVector3D( 1.0f, 0.0f, 0.0f ) );
    orient.rotate( mRotation.y(), QVector3D( 0.0f, 1.0f, 0.0f ) );

    return orient;
}

void GLCamera::walk( float step )
{
    QVector4D forward = orientation().inverted() * QVector4D( 0.0f, 0.0f, -step, 1.0f );

    mPosition += forward.toVector3D();
}

void GLCamera::strafe( float step )
{
    QVector4D right = orientation().inverted() * QVector4D( step, 0.0f, 0.0f, 1.0f );

    mPosition += right.toVector3D();
}

void GLCamera::raise( float step )
{
    QVector4D up = orientation().inverted() * QVector4D( 0.0f, step, 0.0f, 1.0f );

    mPosition += up.toVector3D();
}

void GLCamera::rotateX( float angDeg )
{
    mRotation.setX( angDeg );
}

void GLCamera::rotateY( float angDeg )
{
    mRotation.setY( angDeg );
}

void GLCamera::updateView( void )
{
    mView = orientation();
    mView.translate( -mPosition );
}

void GLCamera::setPerspective( float fovy, float aspect, float zNear, float zFar )
{
    mProjection.perspective( fovy, aspect, zNear, zFar );
}

void GLCamera::reset( void )
{
    mPosition = QVector3D( 0.0f, 0.0f, 0.0f );
    mRotation = QVector3D( 0.0f, 0.0f, 0.0f );

    mView.setToIdentity();
}
