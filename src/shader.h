#pragma once

#include "common.h"

GLuint LinkProgram( GLuint shaders[], int len );

GLuint CompileShader( const char* filepath, GLenum type );

GLuint CompileShaderSource( const char* src, int length, GLenum type ); 
