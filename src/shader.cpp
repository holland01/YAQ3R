#include "shader.h"
#include "log.h"
#include "glutil.h"

GLuint LinkProgram( GLuint shaders[], int len )
{
    GLuint program = glCreateProgram();

    for ( int i = 0; i < len; ++i )
        glAttachShader( program, shaders[ i ] );

    glLinkProgram( program );

    GLint linkSuccess;
    glGetProgramiv( program, GL_LINK_STATUS, &linkSuccess );

    if ( !linkSuccess )
    {
        GLint logLen;
        glGetProgramiv( program, GL_INFO_LOG_LENGTH, &logLen );

        char* infoLog = new char[ logLen ]();
        glGetProgramInfoLog( program, logLen, NULL, infoLog );

        MLOG_ERROR( "GLSL LINK MLOG_ERROR: %s", infoLog );

		delete[] infoLog;
    }

    for ( int i = 0; i < len; ++i )
    {
        glDetachShader( program, shaders[ i ] );
        glDeleteShader( shaders[ i ] );
    }

    return program;
}

// (Slightly modified) Implementation is copy-pasta from http://code.google.com/p/openglbook-samples/source/browse/trunk/Chapter%204/Utils.c
GLuint CompileShader( const char* filename, GLenum shader_type )
{
    GLuint shaderId = 0;
    FILE* file;
    long file_size = -1;
    char* glsl_source;

    if (NULL != (file = fopen(filename, "rb")) &&
            0 == fseek(file, 0, SEEK_END) &&
            -1 != (file_size = ftell(file)))
    {
        rewind(file);

        if (NULL != (glsl_source = (char*)malloc(file_size + 1)))
        {
            if (file_size == (long)fread(glsl_source, sizeof(char), file_size, file))
            {
                glsl_source[file_size] = '\0';

                shaderId = CompileShaderSource( glsl_source, shader_type );
            }
            else
            {
                MLOG_ERROR( "ERROR: Could not read file %s\n", filename );
            }

            free( glsl_source );
        }
        else
        {
            MLOG_ERROR( "ERROR: Could not allocate %ld bytes.\n", file_size );
        }

        fclose(file);
    }
    else
    {
        MLOG_ERROR( "ERROR: Could not open file %s\n", filename );
    }

    return shaderId;
}

GLuint CompileShaderSource( const char* src, GLenum type )
{
	GLuint shaderId;
	GL_CHECK(shaderId = glCreateShader(type));
	if (0 != shaderId)
    {
        // necessary to avoid -Werror raise on incompatible pointer type, when passed to glShaderSource
        const char* sourceconst = src;
        //int length[ 1 ];
        //length[ 0 ] = strlen( src );

        glShaderSource( shaderId, 1, &sourceconst, NULL );
        glCompileShader( shaderId );

        GLint compileSuccess;
        glGetShaderiv( shaderId, GL_COMPILE_STATUS, &compileSuccess );

        if ( compileSuccess == GL_FALSE )
        {
            GLint logLen;
            glGetShaderiv( shaderId, GL_INFO_LOG_LENGTH, &logLen );

            char* infoLog = new char[ logLen ]();
            infoLog[ logLen ] = '\0';

            glGetShaderInfoLog( shaderId, logLen, NULL, infoLog );

            MLOG_ERROR( "SHADER COMPILE MLOG_ERROR: %s", infoLog );
        }
    }
    else
    {
        MLOG_ERROR( "ERROR: Could not create a shader.\n" );
    }

	return shaderId;
}
