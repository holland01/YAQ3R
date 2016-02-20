#pragma once

#pragma once

#define TEXNAME_CHAR_LIMIT 64
#define G_UNSPECIFIED 0xFFFFFFFF
#define G_INTERNAL_BPP 4 // Just to let everyone know we only care really about RGBA... (most of the time)

#define G_INTERNAL_RGBA_FORMAT GL_RGBA
#define G_RGBA_FORMAT GL_RGBA
#define G_INTERNAL_BYTE_FORMAT GL_ALPHA
#define G_BYTE_FORMAT GL_ALPHA
#define G_MAG_FILTER GL_LINEAR

#define G_MIPMAPPED false

#ifdef EMSCRIPTEN
#   define glClearDepth glClearDepthf
#endif

