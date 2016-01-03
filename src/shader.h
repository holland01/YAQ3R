#pragma once

#include "common.h"

GLuint LinkProgram( GLuint shaders[], int len, const std::vector< std::string >& bindAttribs = std::vector< std::string >() );

GLuint CompileShader( const char* filepath, GLenum type );

GLuint CompileShaderSource( const char* src, int length, GLenum type ); 
