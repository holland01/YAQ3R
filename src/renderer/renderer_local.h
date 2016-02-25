#pragma once

#include "common.h"

#define TEXNAME_CHAR_LIMIT 64
#define G_UNSPECIFIED 0xFFFFFFFF
#define G_INTERNAL_BPP 4 // Just to let everyone know we only care really about RGBA... (most of the time)

#define G_INDEX_BYTE_STRIDE 4

#if USE_CORE
#	define G_INTERNAL_RGBA_FORMAT GL_RGBA8
#	define G_RGBA_FORMAT GL_RGBA
#	define G_INTERNAL_BYTE_FORMAT GL_R8
#	define G_BYTE_FORMAT GL_R
#	define G_API_MAJOR_VERSION 3
#	define G_API_MINOR_VERSION 3
#else
#	define G_INTERNAL_RGBA_FORMAT GL_RGBA
#	define G_RGBA_FORMAT GL_RGBA
#	define G_INTERNAL_BYTE_FORMAT GL_ALPHA
#	define G_BYTE_FORMAT GL_ALPHA
#	define G_API_MAJOR_VERSION 2
#	define G_API_MINOR_VERSION 0
#endif

#define G_MAG_FILTER GL_LINEAR

#define G_MIPMAPPED false

#ifdef EMSCRIPTEN
#   define glClearDepth glClearDepthf
#endif

