#include "texture.h"
#include "test_util.h"

#include "../log.h"
#include "../renderer.h"
#include "../shader.h"
#include "../input.h"
#include "../exp/program_batch.h"

namespace {

const int NUM_SPOTLIGHT_VBOS = 2;

enum
{
    MODE_TEX_2D = 0,
    MODE_RENDER_NORMALS
};

const int TEX_WIDTH = 64;
const int TEX_HEIGHT = 64;

const int NUM_BUFFS = 3;

byte checkerboard[ TEX_WIDTH ][ TEX_HEIGHT ][ 4 ];

GLuint buffObjs[ NUM_BUFFS ], texture;

GLuint vaos[] =
{
    0,
    0
};

Pipeline pipeline;

glm::mat4 cubeModel = glm::scale( glm::mat4( 1.0f ), glm::vec3( 10.0f ) );

const glm::mat4& testRotMatrix = glm::rotate( glm::mat4( 1.0f ), glm::radians( 1.0f ), glm::vec3( 1.0f, 1.0f, 0.0f ) );

const float LIGHT_POS_STEP = 3.0f;
glm::vec3 lightPos( 5.0f, 5.0f, 0.0f );

InputCamera camera;

bool cursorVisible, drawNormals;

inline void UpdateMatrices( GLuint program )
{
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_FALSE, glm::value_ptr( cubeModel ) );
    glUniformMatrix4fv( glGetUniformLocation( program, "view" ), 1, GL_FALSE, glm::value_ptr( camera.ViewData().transform ) );
}

} // end namespace

void TEX_HandleKeyInput( GLFWwindow* w, int key, int scancode, int action, int mods )
{
    OnKeyPress( w, key, scancode, action, mods, camera, cursorVisible );

    if ( action == GLFW_PRESS )
    {
        switch( key )
        {
            case GLFW_KEY_J:
                lightPos.y += LIGHT_POS_STEP;
                break;
            case GLFW_KEY_K:
                lightPos.y -= LIGHT_POS_STEP;
                break;
            case GLFW_KEY_L:
                lightPos.x += LIGHT_POS_STEP;
                break;
            case GLFW_KEY_H:
                lightPos.x -= LIGHT_POS_STEP;
                break;
            case GLFW_KEY_I:
                lightPos.z -= LIGHT_POS_STEP;
                break;
            case GLFW_KEY_O:
                lightPos.z += LIGHT_POS_STEP;
                break;
        }

        if ( ( key >= 74 && key <= 76 ) || key == 72 )
        {
            MyPrintf( "Light Position", "x: %f\n y: %f\n z: %f", lightPos.x, lightPos.y, lightPos.z );
        }
    }
}

void TEX_HandleMouseMove( GLFWwindow* w, double x, double y )
{
    camera.EvalMouseMove( ( float ) x, ( float ) y );
}

void TEX_LoadTest( GLFWwindow* window )
{
    glClearColor( 0.0f, 0.0f, 0.3f, 1.0f );

    glfwSetKeyCallback( window, TEX_HandleKeyInput );
    glfwSetCursorPosCallback( window, TEX_HandleMouseMove );
    glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_NORMAL );

    cursorVisible = true;
    drawNormals = true;

    pipeline.Alloc();

    // Generate our texture colors
    for ( int i = 0; i < TEX_WIDTH; ++i )
    {
        for ( int j = 0; j < TEX_HEIGHT; ++j )
        {
            unsigned char v = ( ( ( ( i & 0x8 ) == 0 ) ^ ( ( j & 0x8 ) == 0 ) ) * 255 ) & 128;

            checkerboard[ i ][ j ][ 0 ] = v;
            checkerboard[ i ][ j ][ 1 ] = v;
            checkerboard[ i ][ j ][ 2 ] = v;
            checkerboard[ i ][ j ][ 3 ] = v;
        }
    }

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

    int numNorms = 3 * 6 * 6;

    float normalData[ numNorms ];

    int i, j;


    // Compute vertex normals
    for ( i = 0, j = 0; i < SIGNED_LEN( vertexData ); i += 15, j += 3 )
    {
        glm::vec3 vertNorm( 0.0f );

        //float avg = 0.0f;

        // First get surface normals for every 3 verts.
        // This is performed so we can add the surface normal to our current vertex normal

        glm::vec3
            v1( vertexData[ i ], vertexData[ i + 1 ], vertexData[ i + 2 ] ),
            v2( vertexData[ i + 5 ], vertexData[ i + 6], vertexData[ i + 7 ] ),
            v3( vertexData[ i + 10 ], vertexData[ i + 11 ], vertexData[ i + 12 ] );

        vertNorm = glm::normalize( v1 + v2 + v3 );

        normalData[ j     ] = vertNorm.x;
        normalData[ j + 1 ] = vertNorm.y;
        normalData[ j + 2 ] = vertNorm.z;
    }

    pipeline.SetGlobalMat( GM4_PROJECTION, camera.ViewData().projection );

    {
        GLuint tex2D = []( void ) -> GLuint
        {
            GLuint shaders[] =
            {
                CompileShader( "src/tex2D.vert", GL_VERTEX_SHADER ),
                CompileShader( "src/tex2D.frag", GL_FRAGMENT_SHADER ),
                CompileShader( "src/shared_matrix.glsl", GL_VERTEX_SHADER ),
                CompileShader( "src/shared_matrix.glsl", GL_FRAGMENT_SHADER )
            };

            return LinkProgram( shaders, 4 );
        }();

        GLuint singleColor = []( void ) -> GLuint
        {
            GLuint shaders[] =
            {
                CompileShader( "src/baseVertex.vert", GL_VERTEX_SHADER ),
                CompileShader( "src/singleColor.frag", GL_FRAGMENT_SHADER ),
                CompileShader( "src/shared_matrix.glsl", GL_VERTEX_SHADER )
            };

            return LinkProgram( shaders, 3 );
        }();

        glBindAttribLocation( tex2D, 0, "position" );
        glBindAttribLocation( tex2D, 1, "uv" );
        glBindAttribLocation( tex2D, 2, "normal" );

        pipeline.AddProgram( "tex2D", tex2D,
                            { "position", "uv", "normal" },
                            { "normalMatrix", "lightPosition", "lightIntensity", "lightColor", "sampler" } );

        glBindAttribLocation( singleColor, 0, "position" );

        pipeline.AddProgram( "singleColor", singleColor,
                            { "position" },
                            { "fragmentColor" } );
    }

    glGenVertexArrays( 2, vaos );
    glBindVertexArray( vaos[ MODE_TEX_2D ] );
    {
        pipeline.UseProgram( "tex2D" );

        glGenBuffers( NUM_BUFFS, buffObjs );
        glBindBuffer( GL_ARRAY_BUFFER, buffObjs[ 0 ] );
        glBufferData( GL_ARRAY_BUFFER, sizeof( vertexData ), vertexData, GL_STATIC_DRAW );

        glEnableVertexAttribArray( pipeline.Attrib( "position" ) );
        glEnableVertexAttribArray( pipeline.Attrib( "uv" ) );
        glVertexAttribPointer( pipeline.Attrib( "position" ), 3, GL_FLOAT, GL_FALSE, sizeof( float ) * 5, 0 );
        glVertexAttribPointer( pipeline.Attrib( "uv" ), 2, GL_FLOAT, GL_FALSE, sizeof( float ) * 5, ( void* )( sizeof( float ) * 3 ) );

        glBindBuffer( GL_ARRAY_BUFFER, buffObjs[ 1 ] );
        glBufferData( GL_ARRAY_BUFFER, sizeof( normalData ), normalData, GL_STATIC_DRAW );

        glEnableVertexAttribArray( pipeline.Attrib( "normal" ) );
        glVertexAttribPointer( pipeline.Attrib( "normal" ), 3, GL_FLOAT, GL_FALSE, sizeof( float ) * 3, ( void* ) 0 );

        pipeline.SetUniformVec( "lightColor", glm::vec4( 1.0f, 1.0f, 1.0f, 1.0f ) );
        pipeline.SetUniformScalar( "lightIntensity", 1.5f );
    }
    glBindVertexArray( 0 );

    float normVertData[ numNorms ];

    for ( j = 0; j < SIGNED_LEN( vertexData ); j += 3 )
    {
        normVertData[ j ] =     normalData[ j     ];
        normVertData[ j + 1 ] = normalData[ j + 1 ];
        normVertData[ j + 2 ] = normalData[ j + 2 ];

        for ( int n = 0; n < 6; ++n )
            normVertData[ n + j ] *= 1.5f;
    }

    glBindVertexArray( vaos[ MODE_RENDER_NORMALS ] );
    {
        pipeline.UseProgram( "singleColor" );

        glBindBuffer( GL_ARRAY_BUFFER, buffObjs[ 2 ] );
        glBufferData( GL_ARRAY_BUFFER, sizeof( normVertData ), normVertData, GL_STATIC_DRAW );

        glEnableVertexAttribArray( pipeline.Attrib( "position" ) );
        glVertexAttribPointer( pipeline.Attrib( "position" ), 3, GL_FLOAT, GL_FALSE, sizeof( float ) * 3, ( void* ) 0 );

        pipeline.SetUniformVec( "fragmentColor", glm::vec4( 0.0f, 1.0f, 0.0f, 1.0f  ) );
        pipeline.ReleaseProgram();
    }
    glBindVertexArray( 0 );

    glGenTextures( 1, &texture);
    glBindTexture( GL_TEXTURE_2D, texture );
    {
        glTexStorage2D( GL_TEXTURE_2D, 4, GL_RGBA8, TEX_WIDTH, TEX_HEIGHT );
        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, TEX_WIDTH, TEX_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, ( void* )checkerboard );

        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

        glBindTexture( GL_TEXTURE_2D, 0 );
    }
}

void TEX_DrawTest( void )
{
    camera.Update();

    pipeline.SetGlobalMat( GM4_MODELVIEW, camera.ViewData().transform * cubeModel );
    pipeline.SetGlobalMat( GM3_MODELVIEW_NORM, cubeModel );

    cubeModel = testRotMatrix * cubeModel;

    pipeline.UseProgram( "tex2D" );
    {
        pipeline.SetUniformVec( "lightPosition", lightPos );
        pipeline.SetUniformScalar( "sampler", 0 );

        glBindVertexArray( vaos[ MODE_TEX_2D ] );
        glBindBuffer( GL_ARRAY_BUFFER, buffObjs[ 2 ] );

        glActiveTexture( GL_TEXTURE0 );
        glBindTexture( GL_TEXTURE_2D, texture );

        glDrawArrays( GL_TRIANGLES, 0, 36 );
        glBindTexture( GL_TEXTURE_2D, 0 );
    }

    if ( drawNormals )
    {
        pipeline.UseProgram( "singleColor" );
        {
            glBindVertexArray( vaos[ MODE_RENDER_NORMALS ] );
            glBindBuffer( GL_ARRAY_BUFFER, buffObjs[ 2 ] );

            glDrawArrays( GL_POINTS, 0, 36 );
        }
    }

    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindVertexArray( 0 );

    pipeline.ReleaseProgram();
}

