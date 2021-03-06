QT -= core gui

QMAKE_CXXFLAGS += -std=c++1y -Wall -Werror


#for stb_image.c lib
QMAKE_CXXFLAGS += \
-Wno-unused-but-set-variable \
-Wno-missing-field-initializers \
-Wno-sign-compare

debug {
   DEFINES += DEBUG
   QMAKE_CXXFLAGS += -Wno-unused-function -Wno-unused-parameter -Wno-unused-variable -Wno-multi-line-comment
   QMAKE_CXXFLAGS += -O0
}

unix {
    INCLUDEPATH += $$PWD/src
    INCLUDEPATH += /home/amsterdam/include/glm
    LIBS += -L/usr/lib64 -lGL -lGLU -lGLEW -lglfw -lX11 -lXxf86vm -lXrandr -lpthread -lXi
}

HEADERS += \
    src/common.h \
    src/vec.h \
    src/shader.h \
    src/renderer.h \
    src/global.h \
    src/gldebug.h \
    src/tests/trenderer.h \
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
    src/glutil.h \
    src/bsp_data.h \
    src/render_data.h \
    src/io.h \
    src/renderer/texture.h \
    src/tests/test_textures.h \
    src/renderer/buffer.h \
    src/renderer/renderer_local.h \
    src/renderer/shared.h \
    src/renderer/util.h

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
    src/renderer.cpp \
    src/main.cpp \
    src/gldebug.cpp \
    src/tests/trenderer.cpp \
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
    src/glutil.cpp \
    src/bsp_data.cpp \
    src/render_data.cpp \
    src/io.cpp \
    src/renderer/texture.cpp \
    src/tests/test_textures.cpp \
    src/renderer/buffer.cpp \
    src/renderer/util.cpp

DISTFILES += \
    remember.todo \
    src/debug.frag \
    src/debug.vert \
    src/main_es.vert \
    src/main_es.frag

