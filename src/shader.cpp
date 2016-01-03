#include "shader.h"
#include "io.h"
#include "glutil.h"

GLuint LinkProgram( GLuint shaders[], int len, const std::vector< std::string >& bindAttribs )
{
    GLuint program;
    GL_CHECK( program = glCreateProgram() );

    for ( int i = 0; i < len; ++i )
        GL_CHECK( glAttachShader( program, shaders[ i ] ) );

    for ( uint32_t i = 0; i < bindAttribs.size(); ++i )
        GL_CHECK( glBindAttribLocation( program, i, bindAttribs[ i ].c_str() ) );

    GL_CHECK( glLinkProgram( program ) );

    GLint linkSuccess;
    GL_CHECK( glGetProgramiv( program, GL_LINK_STATUS, &linkSuccess ) );

    if ( !linkSuccess )
    {
        GLint logLen;
        GL_CHECK( glGetProgramiv( program, GL_INFO_LOG_LENGTH, &logLen ) );

        char* infoLog = new char[ logLen ]();
        GL_CHECK( glGetProgramInfoLog( program, logLen, NULL, infoLog ) );

        MLOG_ERROR( "GLSL LINK MLOG_ERROR: %s", infoLog );

		delete[] infoLog;
    }

    for ( int i = 0; i < len; ++i )
    {
        GL_CHECK( glDetachShader( program, shaders[ i ] ) );
        GL_CHECK( glDeleteShader( shaders[ i ] ) );
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

                shaderId = CompileShaderSource( glsl_source, file_size, shader_type );
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

GLuint CompileShaderSource( const char* src, const int length, GLenum type )
{
	GLuint shaderId;
	GL_CHECK(shaderId = glCreateShader(type));
	if (0 != shaderId)
    {
        if ( length > 0 )
		{
			int blength[ 1 ] = { length };
			glShaderSource( shaderId, 1, &src, blength );
		}
		else
		{
			glShaderSource( shaderId, 1, &src, NULL );
		}

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

            MLOG_ERROR( "SHADER COMPILE MLOG_ERROR [ %s ]: %s", ( type == GL_VERTEX_SHADER ) ? "vertex" : "fragment", infoLog );
        }
    }
    else
    {
        MLOG_ERROR( "ERROR: Could not create a shader.\n" );
    }

	return shaderId;
}
