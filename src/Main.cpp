#include <QApplication>
#include "GLWidget.h"

int main( int argc, char** argv )
{
    QApplication app( argc, argv );

    QGLWidget* widget = new GLWidget;

    app.setActiveWindow( widget );

    widget->show();

    return app.exec();
}


