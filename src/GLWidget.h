#pragma once

#include "common.h"
#include "GLRenderer.h"

class GLWidget
{   
public:

    GLWidget( void );
    ~GLWidget( void );

private:

    void initializeGL( void );
    void resizeGL( int width, int height );
    void paintGL( void );
    void updateGL( void );


    void keyPressEvent(int key );

    GLRenderer          mRenderer;
};


