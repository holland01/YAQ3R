#pragma once

/*
============================

Author: Holland Schutte
License: WTFPL

    gldebug.h

    Base logging functionality
    relevant to OpenGL.

============================
*/

#include "common.h"


void            glDebugInit( void );

void            glDebugKill( void );

void  GL_PROC	glDebugOutProc( GLenum source,
                            GLenum type,
                            GLuint id,
                            GLenum severity,
                            GLsizei length,
                            const GLchar* message,
                            const void* userParam );

