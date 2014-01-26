#include "texture.h"
#include "../renderer.h"
#include "../shader.h"

// CREATE SIX TEXTURES FOR EACH FACE

const unsigned char COLOR_A = 0xFF;
const unsigned char COLOR_B = 0x00;

const unsigned char checkerboardTexture[] =
{
    /*
    COLOR_A, COLOR_B, COLOR_A, COLOR_B, COLOR_A, COLOR_B, COLOR_A, COLOR_B,
    COLOR_B, COLOR_A, COLOR_B, COLOR_A, COLOR_B, COLOR_A, COLOR_B, COLOR_A,
    COLOR_A, COLOR_B, COLOR_A, COLOR_B, COLOR_A, COLOR_B, COLOR_A, COLOR_B,
    COLOR_B, COLOR_A, COLOR_B, COLOR_A, COLOR_B, COLOR_A, COLOR_B, COLOR_A,
    COLOR_A, COLOR_B, COLOR_A, COLOR_B, COLOR_A, COLOR_B, COLOR_A, COLOR_B,
    COLOR_B, COLOR_A, COLOR_B, COLOR_A, COLOR_B, COLOR_A, COLOR_B, COLOR_A,
    COLOR_A, COLOR_B, COLOR_A, COLOR_B, COLOR_A, COLOR_B, COLOR_A, COLOR_B,
    COLOR_B, COLOR_A, COLOR_B, COLOR_A, COLOR_B, COLOR_A, COLOR_B, COLOR_A,
        */

    COLOR_A, COLOR_B, COLOR_A, COLOR_B, COLOR_A, COLOR_B, COLOR_A, COLOR_B,
    COLOR_B, COLOR_A, COLOR_B, COLOR_A, COLOR_B, COLOR_A, COLOR_B, COLOR_A,
    COLOR_A, COLOR_B, COLOR_A, COLOR_B, COLOR_A, COLOR_B, COLOR_A, COLOR_B,
    COLOR_B, COLOR_A, COLOR_B, COLOR_A, COLOR_B, COLOR_A, COLOR_B, COLOR_A,
    COLOR_A, COLOR_B, COLOR_A, COLOR_B, COLOR_A, COLOR_B, COLOR_A, COLOR_B,
    COLOR_B, COLOR_A, COLOR_B, COLOR_A, COLOR_B, COLOR_A, COLOR_B, COLOR_A,
    COLOR_A, COLOR_B, COLOR_A, COLOR_B, COLOR_A, COLOR_B, COLOR_A, COLOR_B,
    COLOR_B, COLOR_A, COLOR_B, COLOR_A, COLOR_B, COLOR_A, COLOR_B, COLOR_A,
};

GLuint vao, vbo, texture;

GLuint program;

glm::mat4 cubeModel = glm::scale( glm::mat4( 1.0f ), glm::vec3( 10.0f ) );

const glm::mat4& rotMatrix = glm::rotate( glm::mat4( 1.0f ), glm::radians( 1.0f ), glm::vec3( 1.0f, 1.0f, 0.0f ) );

GLCamera camera;

void handleInputTestTexture( GLFWwindow* w, int key, int scancode, int action, int mods )
{
    if ( action == GLFW_PRESS )
    {
        switch ( key )
        {
            case GLFW_KEY_ESCAPE:
                flagExit();
                break;
            default:
                camera.evalKeyPress( key );
                break;
        }
    }


}

void handleMousePosTestTexture( GLFWwindow* w, double x, double y )
{
    camera.evalMouseCoords( ( float ) x, ( float ) y );
}

void loadTestTexture( void )
{
     GLuint shaders[] =
    {
        loadShader( "src/tex2D.vert", GL_VERTEX_SHADER ),
        loadShader( "src/tex2D.frag", GL_FRAGMENT_SHADER )
    };

    GLfloat vertexData[] =
    {
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
         0.5f,  0.5f, -0.5f, 1.0f, 1.0f,
         0.5f,  0.5f, -0.5f, 1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f, 0.0f, 0.0f,
         0.5f, -0.5f,  0.5f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f, 1.0f, 1.0f,
         0.5f,  0.5f,  0.5f, 1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, 0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f, 1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, 1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, 0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, 1.0f, 0.0f,

         0.5f,  0.5f,  0.5f, 1.0f, 0.0f,
         0.5f,  0.5f, -0.5f, 1.0f, 1.0f,
         0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
         0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
         0.5f, -0.5f,  0.5f, 0.0f, 0.0f,
         0.5f,  0.5f,  0.5f, 1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
         0.5f, -0.5f, -0.5f, 1.0f, 1.0f,
         0.5f, -0.5f,  0.5f, 1.0f, 0.0f,
         0.5f, -0.5f,  0.5f, 1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f, 0.0f, 1.0f,
         0.5f,  0.5f, -0.5f, 1.0f, 1.0f,
         0.5f,  0.5f,  0.5f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f, 1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, 0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, 0.0f, 1.0f,

        -1.0f, -1.0f, -0.5f, 0.0f, 0.0f,
         1.0f, -1.0f, -0.5f, 1.0f, 0.0f,
         1.0f,  1.0f, -0.5f, 1.0f, 1.0f,
         1.0f,  1.0f, -0.5f, 1.0f, 1.0f,
        -1.0f,  1.0f, -0.5f, 0.0f, 1.0f,
        -1.0f, -1.0f, -0.5f, 0.0f, 0.0f
    };

    program = makeProgram( shaders, 2 );

    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );

    glGenBuffers( 1, &vbo );
    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    glBufferData( GL_ARRAY_BUFFER, sizeof( vertexData ), vertexData, GL_STATIC_DRAW );

    GLint attribPosition = glGetAttribLocation( program, "position" );
    GLint attribUV       = glGetAttribLocation( program, "uv" );

    glEnableVertexAttribArray( attribPosition );
    glEnableVertexAttribArray( attribUV );
    glVertexAttribPointer( attribPosition, 3, GL_FLOAT, GL_FALSE, sizeof( float ) * 5, 0 );
    glVertexAttribPointer( attribUV, 2, GL_FLOAT, GL_FALSE, sizeof( float ) * 5, ( void* )( sizeof( float ) * 3 ) );

    glBindVertexArray( 0 );

    glGenTextures( 1, &texture);
    glBindTexture( GL_TEXTURE_2D, texture );

    glTexStorage2D( GL_TEXTURE_2D, 2, GL_R3_G3_B2, 8, 8 );
    glTexSubImage1D( GL_TEXTURE_2D, 0, 0, 0, GL_RGB, GL_UNSIGNED_BYTE, ( void* )checkerboardTexture );

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    glBindTexture( GL_TEXTURE_2D, 0 );

    camera.setPerspective( 45.0f, 16.0f / 9.0f, 0.1f, 100.0f );
}

void drawTestTexture( void )
{

    glUseProgram( program );
    glBindVertexArray( vao );
    glBindBuffer( GL_ARRAY_BUFFER, vbo );

    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, texture );

    glUniform1i( glGetUniformLocation( program, "sampler" ), 0 );

    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_FALSE, glm::value_ptr( cubeModel ) );
    glUniformMatrix4fv( glGetUniformLocation( program, "view" ), 1, GL_FALSE, glm::value_ptr( camera.view() ) );
    glUniformMatrix4fv( glGetUniformLocation( program, "projection" ), 1, GL_FALSE, glm::value_ptr( camera.projection() ) );

    glDrawArrays( GL_TRIANGLES, 0, 36 );

    glBindTexture( GL_TEXTURE_2D, 0 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );

    glBindVertexArray( 0 );
    glUseProgram( 0 );

    camera.updateView();
    cubeModel *= rotMatrix;
}
