#pragma once

/*
============================

    gldebug.h

    Base logging functionality
    relevant to OpenGL.

============================
*/

#include "common.h"

typedef enum sys_glio_param_e
{
    GLDEBUG_PARAM_LOG = 0
}
sys_glio_param_t;

typedef enum sys_glio_value_e
{
    GLDEBUG_LOG_STDOUT = 0x1,

    GLDEBUG_LOG_FILE = 0x2
}
sys_glio_value_t;

void            glDebugInit( void );

void            glDebugKill( void );

void            glDebugMapParam( sys_glio_param_t param, sys_glio_value_t value );

GL_PROC void    glDebugOutProc( GLenum source,
                                GLenum type,
                                GLuint id,
                                GLenum severity,
                                GLsizei length,
                                const GLchar* message,
                                void* userParam );

