#pragma once

#include "common.h"

/*
=====================================================

Author: Holland Schutte
License: WTFPL

                   shader.h

    Global GLSL Shader Program helper functions.

    Both LinkProgram and CompileShader will output
    linker or compilation errors, respectively.

=====================================================
*/

GLuint LinkProgram( GLuint shaders[], int len );

GLuint CompileShader( const char* filepath, GLenum type );

GLuint CompileShaderSource( const char* src, GLenum type ); 
