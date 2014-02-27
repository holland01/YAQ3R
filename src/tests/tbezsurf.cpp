#include "tbezsurf.h"
#include "../shader.h"

TBezSurface::TBezSurface( void )
    : Test( 1366, 768 )
{
}

TBezSurface::~TBezSurface( void )
{
    glUseProgram( 0 );
    glBindVertexArray( 0 );

    glDeleteBuffers( 1, &vbo );
    glDeleteVertexArrays( 1, &vao );
    glDeleteProgram( program );
}

void TBezSurface::Load( void )
{
    if ( !Test::Load( "Bezier Surface" ) )
        return;

    glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );

    camPtr = new InputCamera();

    const float pi = glm::pi< float >();

    for ( int i = 0; i < TBEZ_LEVEL + 1; ++i )
    {
        for ( int j = 0; j < TBEZ_LEVEL + 1; ++j )
        {
            float x =  float( i * TBEZ_LEVEL + j ) / float( TBEZ_LEVEL ) * pi;
            float y =  float( j * TBEZ_LEVEL + i ) / float( TBEZ_LEVEL ) * pi;

            controlPoints[ i ][ j ] = glm::vec3( x, 0.0f, glm::normalize( x ) );
        }
    }

    const int L = 10;
    const int Np = 5000;

    vertices.reserve( Np );

    float fLevel = float( TBEZ_LEVEL );

    for ( int p = 0; p < Np; ++p )
    {
        float t = ( float )p / Np;
        float s = 1.0f - t;

        glm::vec3 u( s ), v( t );

        for ( int i = 0; i < TBEZ_LEVEL; ++i )
        {
            float fi = float( i );

            const glm::vec3& e =
                    glm::vec3( fi, fLevel, fLevel ) *
                    glm::pow( t, fi ) * glm::pow( 1.0f - t, fLevel - fi );

            v += e;

            for ( int j = 0; j < TBEZ_LEVEL; ++j )
            {
                float fj = float( j );

                const glm::vec3& d =
                        controlPoints[ i ][ j ] *
                        glm::vec3( fj, fLevel, fLevel ) *
                        glm::pow( s, fj ) * glm::pow( 1.0f - s, fLevel - fj );


                u += d;
            }

            v += u;
        }

        //v = -u;

        color4f_t color;

        color.r = t;
        color.g = color.r + t;
        color.b = 1.0f;
        color.a = 1.0f;

        vec3f_t q, r;

        q.x = u.x;
        q.y = u.y;
        q.z = u.z;

        r.x = v.x;
        r.y = v.y;
        r.z = v.z;

        vertices.push_back( { q, color } );
        vertices.push_back( { r, color } );
    }

    /*
    for ( int p = 0; p < Np; ++p )
    {
        float t = ( float )p / Np;
        float s = 1.0f - t;

        float u = t,
              v = s,
              z;

        for ( int i = 0; i < TBEZ_LEVEL; ++i )
        {
            for ( int j = 0; j < TBEZ_LEVEL; ++j )
            {
                for ( int k = 0; k < TBEZ_LEVEL; ++k )
                {
                    float fi = float( i );
                    float fj = float( j );

                    float d = controlPoints[ i ][ j ].x * glm::pow( s, fj ) * glm::pow( 1.0f - s, fLevel - fj );

                    u += d;

                    float e = controlPoints[ i ][ j ].y * glm::pow( t, fi ) * glm::pow( t - 1.0f, fLevel - fi );

                    v += e;
                }
            }
        }

        z = u * v;

        color4f_t color;

        color.r = t;
        color.g = color.r + t;
        color.b = 1.0f;
        color.a = 1.0f;

        vec3f_t q;

        q.x = u;
        q.y = v;
        q.z = z;

        vertices.push_back( { q, color } );
    }
    */

    program = []( void ) -> GLuint
    {
        GLuint shaders[] =
        {
            CompileShader( "src/main.vert", GL_VERTEX_SHADER ),
            CompileShader( "src/main.frag", GL_FRAGMENT_SHADER )
        };

        return LinkProgram( shaders, 2 );
    }();

    GLint attribPos, attribCol;

    attribPos = glGetAttribLocation( program, "position" );
    attribCol = glGetAttribLocation( program, "color" );

    glUseProgram( program );
    glUniformMatrix4fv( glGetUniformLocation( program, "cameraToClip" ), 1, GL_FALSE, glm::value_ptr( camPtr->ViewData().clipTransform ) );

    glGenBuffers( 1, &vbo );
    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    glBufferData( GL_ARRAY_BUFFER, sizeof( tbezvert_t ) * vertices.size(), &vertices[ 0 ], GL_STATIC_DRAW );

    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );

    // Load vertices
    glEnableVertexAttribArray( attribPos );
    glVertexAttribPointer( attribPos, 3, GL_FLOAT, GL_FALSE, sizeof( tbezvert_t ), ( void* ) offsetof( tbezvert_t, vertex ) );

    // Load colors
    glEnableVertexAttribArray( attribCol );
    glVertexAttribPointer( attribCol, 4, GL_FLOAT, GL_FALSE, sizeof( tbezvert_t ), ( void* ) offsetof( tbezvert_t, color ) );

    glBindVertexArray( 0 );

    glBindBuffer( GL_ARRAY_BUFFER, 0 );
}

const glm::mat4& bezModel = glm::scale( glm::mat4( 1.0f ), glm::vec3( 1.0f ) );

void TBezSurface::Run( void )
{
    camPtr->Update();

    glUniformMatrix4fv( glGetUniformLocation( program, "modelToCamera" ), 1, GL_FALSE, glm::value_ptr( camPtr->ViewData().transform * bezModel ) );

    glBindVertexArray( vao );

    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    glDrawArrays( GL_TRIANGLE_STRIP, 0, vertices.size() );
    glBindVertexArray( 0 );
}
