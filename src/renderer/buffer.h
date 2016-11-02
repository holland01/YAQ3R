#pragma once

#include "common.h"
#include "renderer_local.h"

struct gVertexBufferHandle_t
{
    uint32_t id = G_UNSPECIFIED;
};

void GEnableDepthBuffer( void );

void GLoadVao( void );

gVertexBufferHandle_t GMakeVertexBuffer(
	const std::vector< glm::vec3 >& vertices,
	const std::vector< glm::vec2 >& texCoords = std::vector< glm::vec2 >() );

void GFreeVertexBuffer( gVertexBufferHandle_t& buffer );

void GBindVertexBuffer( const gVertexBufferHandle_t& buffer );

void GReleaseVertexBuffer( void );

struct gIndexBufferHandle_t
{
	uint32_t id = G_UNSPECIFIED;
};

gIndexBufferHandle_t GMakeIndexBuffer( void );

void GFreeIndexBuffer( gIndexBufferHandle_t buffer );

void GPushIndex( gIndexBufferHandle_t buffer, uint32_t v );

uint32_t GGetIndex( gIndexBufferHandle_t buffer, uint32_t index );

void GDrawFromIndices( gIndexBufferHandle_t buffer, GLenum mode );
