#pragma once

#include "common.h"

/*
=====================================================

                   shader.h

    Global GLSL Shader Program helper functions.

    Both LinkProgram and CompileShader will output
    linker or compilation errors, respectively.

=====================================================
*/

GLuint LinkProgram( GLuint shaders[], int len );

GLuint CompileShader( const char* filepath, GLenum type );
