#pragma once

#include "common.h"
#include <unordered_map>

#define TEXNAME_CHAR_LIMIT 64
#define G_UNSPECIFIED 0xFFFFFFFF
#define G_INTERNAL_BPP 4 // Just to let everyone know we only care really about RGBA... (most of the time)
#define G_USE_GL_CORE
#define G_INDEX_BYTE_STRIDE 4

// if 1, we don't use GL_ELEMENT_ARRAY_BUFFER, we just send the indices directly through the draw call itself.
#define G_STREAM_INDEX_VALUES 0

#ifdef G_USE_GL_CORE
#	define G_INTERNAL_RGBA_FORMAT GL_RGBA8
#	define G_RGBA_FORMAT GL_RGBA
#	define G_INTERNAL_BYTE_FORMAT GL_R8
#	define G_BYTE_FORMAT GL_RED
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

using gIndex_t = uint32_t;
using gIndexBuffer_t = std::vector< gIndex_t >;
using gTextureFlags_t = uint32_t;
using gTexSlot_t = int16_t;


#define G_HNULL( handle ) ( ( handle ).id == G_UNSPECIFIED )

template < class Tint >
static bool G_VNULL( Tint v )
{
	return v == ( Tint )G_UNSPECIFIED;
}

using programDataMap_t = std::unordered_map< std::string, GLint >;

#ifdef EMSCRIPTEN
#   define glClearDepth glClearDepthf
#endif
