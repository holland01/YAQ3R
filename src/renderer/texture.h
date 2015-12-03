#pragma once

#include "common.h"

#define TEXNAME_CHAR_LIMIT 64

#ifdef GL_ES
#   define G_INTERNAL_RGBA_FORMAT GL_RGBA
#   define G_RGBA_FORMAT GL_RGBA
#   define G_INTERNAL_BYTE_FORMAT GL_ALPHA
#   define G_BYTE_FORMAT GL_ALPHA
#else
#   define G_INTERNAL_BYTE_FORMAT GL_R8
#   define G_BYTE_FORMAT GL_R
#   define G_INTERNAL_RGBA_FORMAT GL_SRGB8_ALPHA8
#   define G_RGBA_FORMAT GL_SRGB_ALPHA
#endif


enum
{
    TEXTURE_ATLAS = ( 1 << 0 )
};

struct gTextureHandle_t
{
    uint32_t flags;
    const void* data;
};

struct gImageParams_t
{
    bool mipmap = false;
    int32_t width = 0;
    int32_t height = 0;
    int32_t bpp = 0;
    GLenum wrap = GL_REPEAT;
    GLenum minFilter = GL_LINEAR;
    GLenum magFilter = GL_LINEAR;
    GLenum format = G_RGBA_FORMAT;
    GLenum internalFormat = G_INTERNAL_RGBA_FORMAT;
    std::vector< uint8_t > data;
};

gTextureHandle_t GMakeTexture( const gImageParams_t& image, const std::string& name, uint32_t flags );

bool GLoadImageFromFile( const std::string& imagePath, gImageParams_t& image );

bool GDetermineImageFormat( gImageParams_t& image );

bool GSetImageBuffer( gImageParams_t& image, int32_t width, int32_t height, int32_t bpp, uint8_t fillValue );

void GFreeTexture( const gTextureHandle_t& handle );
