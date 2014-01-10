QT += opengl gui core

QMAKE_CXXFLAGS += -std=c++11 -Wall -Werror \
    -Wno-unused-function

LIBS += -lGL -lGLU -lGLEW

HEADERS += \
    src/GLWidget.h \
    src/GLRenderer.h \
    src/common.h \
    src/Quake3Map.h

OTHER_FILES += \
    src/test.vert \
    src/test.frag \
    asset/quake/aty3dm1v2.bsp \
    asset/quake/cp_towers_a4_5.bsp

SOURCES += \
    src/Main.cpp \
    src/GLWidget.cpp \
    src/GLRenderer.cpp \
    src/Quake3Map.cpp

