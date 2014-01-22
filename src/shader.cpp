#include "shader.h"
#include "log.h"

GLuint makeProgram( GLuint shaders[], int len )
{
    GLuint program = glCreateProgram();

    for ( int i = 0; i < len; ++i )
        glAttachShader( program, shaders[ i ] );

    glLinkProgram( program );

    GLint link_success;
    glGetProgramiv( program, GL_LINK_STATUS, &link_success );

    if ( !link_success )
    {
        GLint log_len;
        glGetProgramiv( program, GL_INFO_LOG_LENGTH, &log_len );

        char info_log[ log_len ];
        glGetProgramInfoLog( program, log_len, NULL, info_log );

        ERROR( "GLSL LINK ERROR: %s", info_log );
    }

    for ( int i = 0; i < len; ++i )
    {
        glDetachShader( program, shaders[ i ] );
        glDeleteShader( shaders[ i ] );
    }

    return program;
}

// (Slightly modified) Implementation is copy-pasta from http://code.google.com/p/openglbook-samples/source/browse/trunk/Chapter%204/Utils.c

GLuint loadShader( const char* filename, GLenum shader_type )
{
    GLuint shader_id = 0;
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

                if (0 != (shader_id = glCreateShader(shader_type)))
                {
                    // necessary to avoid -Werror raise on incompatible pointer type, when passed to glShaderSource
                    const char* sourceconst = glsl_source;
                    glShaderSource(shader_id, 1, &sourceconst, NULL);
                    glCompileShader(shader_id);

                    GLint compile_success;
                    glGetShaderiv( shader_id, GL_COMPILE_STATUS, &compile_success );

                    if ( compile_success == GL_FALSE )
                    {
                        GLint log_len;
                        glGetShaderiv( shader_id, GL_INFO_LOG_LENGTH, &log_len );

                        char log[ log_len ];
                        log[ log_len ] = '\0';

                        glGetShaderInfoLog( shader_id, log_len, NULL, log );

                        ERROR( "SHADER COMPILE ERROR: %s", log );
                    }

                }
                else
                {
                    ERROR( "ERROR: Could not create a shader.\n" );
                }
            }
            else
            {
                ERROR( "ERROR: Could not read file %s\n", filename );
            }

            free( glsl_source );
        }
        else
        {
            ERROR( "ERROR: Could not allocate %ld bytes.\n", file_size );
        }

        fclose(file);
    }
    else
    {
        ERROR( "ERROR: Could not open file %s\n", filename );
    }

    return shader_id;
}



