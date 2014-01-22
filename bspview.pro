QMAKE_CXXFLAGS += -std=c++11 -Wall -Werror

debug {
   QMAKE_CXXFLAGS += -Wno-unused-function -Wno-unused-parameter
}

LIBS += -L/usr/lib/regal -lRegal -lglfw -lX11 -lXxf86vm -lXrandr -lpthread -lXi

HEADERS += \
    src/GLWidget.h \
    src/GLRenderer.h \
    src/common.h \
    src/Quake3Map.h \
    src/vec.h

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

