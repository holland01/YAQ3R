import qbs

Product {
    type: "application" // To suppress bundle generation on Mac
    Depends { name: "cpp" }
    consoleApplication: true
    cpp.includePaths: {
        var inc = [
            "/usr/local/include",
            "../src"
        ];

        var librootInc = [
            "/sdl2/include",
        ];

        var devlibRoot = qbs.getEnv("DEVLIB_ROOT");
        librootInc.forEach(function(includePath) {
            inc.push(devlibRoot + includePath);
        });

        return inc;
    }
    cpp.defines: ["__DEBUG_RENDERER__", "DEBUG"]
    cpp.treatWarningsAsErrors: true
    cpp.cxxFlags:[
        "-Wno-self-assign",
        "-Wno-missing-field-initializers",
       // "-Wno-unused-but-set-variable",
        "-Wno-unused-result",
        "-Wno-unused-function",
        "-Wno-strict-aliasing",
        "-std=c++14",
        "-Wno-warn-absolute-paths"
    ]
    cpp.linkerFlags: {
        var linkFlags = [
            "-L" + qbs.getEnv("DEVLIB_ROOT") + "/sdl2/lib/x86_64-linux-gnu/",
            "-L/usr/lib/x86_64-linux-gnu",
            "-lGL",
            "-lGLU",
            "-lGLEW",
            "-L/usr/lib/x86_64-linux-gnu",
            "-lSDL2",
            "-lpthread",
            "-Wl,--no-undefined",
            "-lm",
            "-ldl",
            "-lasound",
            "-lm",
            "-ldl",
            "-lpthread",
            "-lpulse-simple",
            "-lpulse",
            "-lX11",
            "-lXext",
            "-lXcursor",
            "-lXinerama",
            "-lXi",
            "-lXrandr",
            "-lXss",
            "-lXxf86vm",
            "-lwayland-egl",
            "-lwayland-client",
            "-lwayland-cursor",
            "-lxkbcommon",
            "-lpthread",
            "-lrt"
        ];

        return linkFlags;
    }
    files: [
        "../Makefile",
        "../bspviewer.js",
        "../random_notes.md",
        "../src/aabb.cpp",
        "../src/aabb.h",
        "../src/bsp_data.cpp",
        "../src/bsp_data.h",
        "../src/common.h",
        "../src/debug.frag",
        "../src/debug.vert",
        "../src/debug_draw.h",
        "../src/deform.cpp",
        "../src/deform.h",
        "../src/effect_shader.cpp",
        "../src/effect_shader.h",
        "../src/em_api.cpp",
        "../src/em_api.h",
        "../src/eu_ang.cpp",
        "../src/eu_ang.h",
        "../src/frustum.cpp",
        "../src/frustum.h",
        "../src/global.h",
        "../src/glutil.cpp",
        "../src/glutil.h",
        "../src/input.cpp",
        "../src/input.h",
        "../src/io.cpp",
        "../src/io.h",
        "../src/js/em_api.js",
        "../src/lib/atlas_gen.cpp",
        "../src/lib/atlas_gen.h",
        "../src/lib/random.h",
        "../src/lib/stats.h",
        "../src/lightmodel.h",
        "../src/lightsample.frag",
        "../src/main.cpp",
        "../src/main_core.frag",
        "../src/main_core.vert",
        "../src/main_es.frag",
        "../src/main_es.vert",
        "../src/model.h",
        "../src/model.cpp",
        "../src/opengl.h",
        "../src/plane.h",
        "../src/q3bsp.cpp",
        "../src/q3bsp.h",
        "../src/renderer/program.cpp",
        "../src/renderer/program.h",
        "../src/shader.cpp",
        "../src/shader.h",
        "../src/extern/stb_image.c",
        "../src/extern/stb_image.h",
        "../src/lib/circle_buffer.h",
        "../src/lib/math.h",
        "../src/render_data.cpp",
        "../src/render_data.h",
        "../src/renderer.cpp",
        "../src/renderer.h",
        "../src/renderer/buffer.cpp",
        "../src/renderer/buffer.h",
        "../src/renderer/draw_buffer.cpp",
        "../src/renderer/draw_buffer.h",
        "../src/renderer/renderer_local.h",
        "../src/renderer/shared.h",
        "../src/renderer/texture.cpp",
        "../src/renderer/texture.h",
        "../src/renderer/util.cpp",
        "../src/renderer/util.h",
        "../src/tests/test.cpp",
        "../src/tests/test.h",
        "../src/tests/test_atlas_struct.cpp",
        "../src/tests/test_atlas_struct.h",
        "../src/tests/test_textures.cpp",
        "../src/tests/test_textures.h",
        "../src/tests/test_util.h",
        "../src/tests/trenderer.cpp",
        "../src/tests/trenderer.h",
    ]

    Group {
        condition: qbs.buildVariant == "debug"
        cpp.defines: outer.concat(["DEBUG"])
    }

    Group {     // Properties for the produced executable
        fileTagsFilter: product.type
        qbs.install: true
    }

    Group {
        files: [ "../asset/*" ]
        qbs.installDir: "asset"
        qbs.install: true
    }

    Group {
        condition: cpp.compilerName == "emcc_gcc"
        cpp.defines: outer.concat(["EMSCRIPTEN"])
    }
}



