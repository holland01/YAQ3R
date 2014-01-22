#pragma once

#include "common.h"

GLuint makeProgram( GLuint shaders[], int len );

GLuint loadShader( const char* filepath, GLenum type );
