#pragma once

#include "common.h"
#include "renderer_local.h"

#define TEXNAME_CHAR_LIMIT 64
#define G_UNSPECIFIED 0xFFFFFFFF
#define G_INTERNAL_BPP 4 // Just to let everyone know we only care really about RGBA... (most of the time)

#ifdef GLES
#   define G_INTERNAL_RGBA_FORMAT GL_RGBA
#   define G_RGBA_FORMAT GL_RGBA
#   define G_INTERNAL_BYTE_FORMAT GL_R8
#   define G_BYTE_FORMAT GL_R8
#else
#   define G_INTERNAL_BYTE_FORMAT GL_R8
#   define G_BYTE_FORMAT GL_R
#   define G_INTERNAL_RGBA_FORMAT GL_SRGB8_ALPHA8
#   define G_RGBA_FORMAT GL_RGBA
#endif

enum
{
    TEXTURE_ATLAS = ( 1 << 0 ),
};

struct gTextureHandle_t
{
    uint32_t id;
};

struct gSamplerHandle_t
{
    uint32_t id;
};

struct gImageParams_t
{
	uint32_t key = G_UNSPECIFIED; 
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

struct gTextureImage_t
{
    glm::vec2 stOffsetStart;
    glm::vec2 stOffsetEnd;
    glm::vec2 imageScaleRatio;
    glm::vec2 dims;
};

gTextureHandle_t GMakeTexture( const std::vector< gImageParams_t >& images, uint32_t flags );

bool GLoadImageFromFile( const std::string& imagePath, gImageParams_t& image );

bool GDetermineImageFormat( gImageParams_t& image );

bool GSetImageBuffer( gImageParams_t& image, int32_t width, int32_t height, int32_t bpp, uint8_t fillValue );

void GBindTexture( const gTextureHandle_t& handle, uint32_t offset = 0 );

void GReleaseTexture( const gTextureHandle_t& handle, uint32_t offset = 0 );

const gTextureImage_t& GTextureImage( const gTextureHandle_t& handle, uint32_t slot );

const gTextureImage_t& GTextureImageByKey( const gTextureHandle_t& handle, uint32_t key );

glm::vec2 GTextureInverseRowPitch( const gTextureHandle_t& handle );

void GFreeTexture( gTextureHandle_t& handle );
