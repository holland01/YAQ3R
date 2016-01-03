#pragma once

#include "common.h"
#include "texture.h"

struct Program;

void GU_SetupTexParams( const Program& program,
                        const char* uniformPrefix,
                        gTextureHandle_t texHandle,
                        int32_t textureIndex,
                        uint32_t sampler,
                        uint32_t offset = 0 );
