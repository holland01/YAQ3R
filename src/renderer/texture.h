#pragma once

#include "renderer_local.h"

enum
{
	G_TEXTURE_STORAGE_KEY_MAPPED_BIT = 1 << 0,
	G_TEXTURE_DUMMY_BIT = 1 << 31
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
	gSamplerHandle_t sampler;
	int32_t width = 0;
	int32_t height = 0;
	gTextureFlags_t flags = 0;
	std::vector< uint8_t > data;
};

using gImageParamList_t = std::vector< gImageParams_t >;

struct gTextureImage_t
{
	glm::vec2 gridLocation;
	glm::vec2 stOffsetStart;
	glm::vec2 stOffsetEnd;
	glm::vec2 imageScaleRatio;
	glm::vec2 dims;
};

using gTextureImageKey_t = uint8_t;

struct gTextureMakeParams_t
{

	gImageParamList_t& images;
	std::vector< gTextureImageKey_t > keyMaps; // specify G_TEXTURE_STORAGE_KEY_MAPPED
	gSamplerHandle_t sampler;
	gTextureFlags_t flags;

	gTextureMakeParams_t( gImageParamList_t& images_, const gSamplerHandle_t& sampler_, gTextureFlags_t flags_ = 0 )
		: images( images_ ),
		  sampler( sampler_ ),
		  flags( flags_ )
	{
	}
};

using gTextureImageKeyList_t = std::vector< gTextureImageKey_t >;

gSamplerHandle_t GMakeSampler(
	int8_t bpp = G_INTERNAL_BPP,
	bool mipmapped = G_MIPMAPPED,
	uint32_t wrap = GL_CLAMP_TO_EDGE
);

int8_t GSamplerBPP( const gSamplerHandle_t& sampler );

gTextureHandle_t GMakeTexture( gTextureMakeParams_t& makeParams );

bool GLoadImageFromMemory( gImageParams_t& image, const std::vector< uint8_t >& buffer,
 	int32_t width, int32_t height, int32_t bpp );

bool GLoadImageFromFile( const std::string& imagePath, gImageParams_t& image );

bool GSetImageBuffer( gImageParams_t& image, int32_t width, int32_t height,
	uint8_t fillValue );

// Sets the given destImage's data to sourceData's, in a manner which follows
// the user-specified image bpp.
// fetchChannel is only relevant if destImage.bpp == 1
void GSetAlignedImageData( gImageParams_t& destImage,
							const uint8_t* sourceData,
							int8_t sourceBPP,
							uint32_t numPixels,
							uint8_t fetchChannel = 0 );

void GStageSlot( gTexSlot_t slot );

void GUnstageSlot( void );

void GBindTexture( const gTextureHandle_t& handle, uint32_t offset = 0 );

void GReleaseTexture( const gTextureHandle_t& handle, uint32_t offset = 0 );

void GBindGrid( const gTextureHandle_t& handle, uint32_t grid, uint32_t offset = 0 );

const gTextureImage_t& GTextureImage( const gTextureHandle_t& handle, uint32_t slot );

const gTextureImage_t& GTextureImage( const gTextureHandle_t& handle );

uint16_t GTextureImageCount( const gTextureHandle_t& handle );

gTextureImageKeyList_t GTextureImageKeys( const gTextureHandle_t& handle );

uint16_t GTextureGridCount( const gTextureHandle_t& handle );

glm::vec2 GTextureInverseRowPitch( const gTextureHandle_t& handle );

uint16_t GTextureMegaWidth( const gTextureHandle_t& handle );

uint16_t GTextureMegaHeight( const gTextureHandle_t& handle );

bool GValidTextureDimensions( uint16_t width, uint16_t height );

void GFreeTexture( gTextureHandle_t& handle );
