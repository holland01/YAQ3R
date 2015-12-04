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

#define G_HANDLE_INVALID 0xDEADBEEF

struct gTextureHandle_t
{
    uint32_t id;
};

struct gVertexBufferHandle_t
{
    uint32_t id;
};

struct gImageParams_t
{
    bool mipmap = false;
    int32_t width = 0;
    int32_t height = 0;
    int8_t bpp = 0;
    uint32_t wrap = GL_REPEAT;
    uint32_t minFilter = GL_LINEAR;
    uint32_t magFilter = GL_LINEAR;
    uint32_t format = G_RGBA_FORMAT;
    uint32_t internalFormat = G_INTERNAL_RGBA_FORMAT;
    std::vector< uint8_t > data;
};

void GEnableDepthBuffer( void );

gVertexBufferHandle_t GMakeVertexBuffer( const std::vector< glm::vec3 >& vertices );

void GFreeVertexBuffer( gVertexBufferHandle_t& handle );

void GBindVertexBuffer( const gVertexBufferHandle_t& buffer );

void GReleaseVertexBuffer( void );

gTextureHandle_t GMakeTexture( const std::vector< gImageParams_t >& images, uint32_t flags );

bool GLoadImageFromFile( const std::string& imagePath, gImageParams_t& image );

bool GDetermineImageFormat( gImageParams_t& image );

bool GSetImageBuffer( gImageParams_t& image, int32_t width, int32_t height, int32_t bpp, uint8_t fillValue );

void GFreeTexture( gTextureHandle_t& handle );
