QMAKE_CXXFLAGS += -std=c++11 -Wall -Werror

debug {
   QMAKE_CXXFLAGS += -Wno-unused-function -Wno-unused-parameter
}

unix {
    LIBS += -lGL -lGLU -lGLEW -lglfw3 -lX11 -lXxf86vm -lXrandr -lpthread -lXi
}

HEADERS += \
    src/common.h \
    src/vec.h \
    src/shader.h \
    src/log.h \
    src/q3m.h \
    src/renderer.h \
    src/global.h \
    def.h \
    src/gldebug.h \
    src/tests/texture.h \
    src/tests/trenderer.h

OTHER_FILES += \
    src/test.vert \
    src/test.frag \
    asset/quake/aty3dm1v2.bsp \
    asset/quake/cp_towers_a4_5.bsp \
    log/drawLog.log \
    log/camLog.log \
    log/bspData.log \
    log/gl.log \
    src/tex2D.vert \
    src/tex2D.frag

SOURCES += \
    src/shader.cpp \
    src/log.cpp \
    src/q3m.cpp \
    src/renderer.cpp \
    src/main.cpp \
    src/gldebug.cpp \
    src/tests/texture.cpp \
    src/tests/trenderer.cpp

