#pragma once

#include "common.h"
#include "renderer_local.h"

struct gVertexBufferHandle_t
{
    uint32_t id;
};

void GEnableDepthBuffer( void );

gVertexBufferHandle_t GMakeVertexBuffer( const std::vector< glm::vec3 >& vertices, const std::vector< glm::vec2 >& texCoords = std::vector< glm::vec2 >() );

void GFreeVertexBuffer( gVertexBufferHandle_t& handle );

void GBindVertexBuffer( const gVertexBufferHandle_t& buffer );

void GReleaseVertexBuffer( void );
