QT -= core gui

QMAKE_CXXFLAGS += -std=c++11 -Wall -Werror


#for stb_image.c lib
QMAKE_CXXFLAGS += \
-Wno-unused-but-set-variable \
-Wno-missing-field-initializers

debug {
   QMAKE_CXXFLAGS += -Wno-unused-function -Wno-unused-parameter -Wno-unused-variable -Wno-multi-line-comment
}

unix {
    INCLUDEPATH += /home/amsterdam/include/glm
    LIBS += -lGL -lGLU -lGLEW -lglfw3 -lX11 -lXxf86vm -lXrandr -lpthread -lXi
}

HEADERS += \
    src/common.h \
    src/vec.h \
    src/shader.h \
    src/log.h \
    src/renderer.h \
    src/global.h \
    src/gldebug.h \
    src/tests/trenderer.h \
    src/def.h \
    src/q3m_model.h \
    src/math_util.h \
    src/mtrand.h \
    src/input.h \
    src/eu_ang.h \
    src/tests/test_util.h \
    src/q3bsp.h \
    src/frustum.h \
    src/plane.h \
    src/aabb.h \
    src/tests/test.h \
    src/tests/test_tessellation.h \
    src/deform.h \
    src/effect_shader.h \
    src/glutil.h

OTHER_FILES += \
    asset/quake/aty3dm1v2.bsp \
    asset/quake/cp_towers_a4_5.bsp \
    log/drawLog.log \
    log/camLog.log \
    log/bspData.log \
    log/gl.log \
    asset/quake/overkill.bsp \
    asset/quake/railgun_arena/map.bsp \
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
    src/main.vert \
    src/main.frag \
    asset/quake/railgun_arena/maps/Railgun_Arena.aas \
    asset/quake/railgun_arena/maps/Railgun_Arena.bsp \
    asset/quake/railgun_arena/scripts/Railgun_Arena.shader \
    asset/quake/railgun_arena/scripts/Railgun_Arena.arena

SOURCES += \
    src/shader.cpp \
    src/log.cpp \
    src/renderer.cpp \
    src/main.cpp \
    src/gldebug.cpp \
    src/tests/trenderer.cpp \
    src/q3m_model.cpp \
    src/math_util.cpp \
    src/mtrand.cpp \
    src/input.cpp \
    src/eu_ang.cpp \
    src/extern/stb_image.c \
    src/q3bsp.cpp \
    src/frustum.cpp \
    src/aabb.cpp \
    src/tests/test.cpp \
    src/tests/test_tessellation.cpp \
    src/deform.cpp \
    src/effect_shader.cpp \
    src/glutil.cpp

