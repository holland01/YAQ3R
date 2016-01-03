#pragma once

#include "common.h"
#include "renderer_local.h"

struct gConstantBufferHandle_t
{
    uint32_t id;
};

gConstantBufferHandle_t GMakeConstantBuffer( size_t bufferSize );
