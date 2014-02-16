QT -= core gui

QMAKE_CXXFLAGS += -std=c++11 -Wall -Werror

debug {
   QMAKE_CXXFLAGS += -Wno-unused-function -Wno-unused-parameter -Wno-unused-variable
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
    src/gldebug.h \
    src/tests/texture.h \
    src/tests/trenderer.h \
    src/def.h \
    src/q3m_model.h \
    src/math_util.h \
    src/mtrand.h \
    src/input.h \
    src/eu_ang.h \
    src/tests/jpeg.h \
    src/tests/test_util.h \
    src/exp/spotlight.h \
    src/exp/program_batch.h

OTHER_FILES += \
    asset/quake/aty3dm1v2.bsp \
    asset/quake/cp_towers_a4_5.bsp \
    log/drawLog.log \
    log/camLog.log \
    log/bspData.log \
    log/gl.log \
    src/tex2D.vert \
    src/tex2D.frag \
    asset/quake/overkill.bsp \
    src/main.vert \
    src/targetDraw.vert \
    src/vertexColor.frag \
    asset/quake/railgun_arena/map.bsp \
    asset/quake/railgun_arena/map.aas \
    asset/quake/railgun_arena/textures/aedm7/aecell_decal_logo1.tga \
    asset/quake/railgun_arena/textures/aedm7/aecell_decal_spawn1.tga \
    asset/quake/railgun_arena/textures/aedm7/aecell_pads_gl.tga \
    asset/quake/railgun_arena/textures/aedm7/aecell_pads_lg.tga \
    asset/quake/railgun_arena/textures/aedm7/aecell_pads_mh.tga \
    asset/quake/railgun_arena/textures/aedm7/aecell_pads_pg.tga \
    asset/quake/railgun_arena/textures/aedm7/aecell_pads_quad.tga \
    asset/quake/railgun_arena/textures/aedm7/aecell_pads_ra.tga \
    asset/quake/railgun_arena/textures/aedm7/aecell_pads_rg.tga \
    asset/quake/railgun_arena/textures/aedm7/aecell_pads_rl.tga \
    asset/quake/railgun_arena/textures/aedm7/aecell_pads_sg.tga \
    asset/quake/railgun_arena/textures/aedm7/aecell_pads_ya.tga \
    src/shared_matrix.glsl \
    src/baseVertex.vert \
    src/singleColor.frag

SOURCES += \
    src/shader.cpp \
    src/log.cpp \
    src/q3m.cpp \
    src/renderer.cpp \
    src/main.cpp \
    src/gldebug.cpp \
    src/tests/texture.cpp \
    src/tests/trenderer.cpp \
    src/q3m_model.cpp \
    src/math_util.cpp \
    src/mtrand.cpp \
    src/input.cpp \
    src/eu_ang.cpp \
    src/tests/jpeg.cpp \
    src/tests/test_util.cpp \
    src/exp/program_batch.cpp

