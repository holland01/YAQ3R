#pragma once

#include "common.h"
#include <QGLWidget>
#include "GLRenderer.h"

class GLWidget : public QGLWidget
{
    Q_OBJECT

public:

    GLWidget( QWidget* parent = 0 );
    ~GLWidget( void );

protected:

    void initializeGL( void );
    void resizeGL( int width, int height );
    void paintGL( void );
    void updateGL( void );


    void keyPressEvent( QKeyEvent* event );

private:

    GLRenderer          mRenderer;
};


