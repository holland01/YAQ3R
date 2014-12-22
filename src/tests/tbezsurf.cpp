#include "tbezsurf.h"
#include "../shader.h"
#include "../log.h"
#include <algorithm>
#include <math.h>

static float Factorial( float n )
{
	if ( n == 1 || n == 0 )
		return 1;
	
	return Factorial( n - 1 ) * n;
}

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

	glm::vec3 points[ 9 ] = 
	{
		glm::vec3( 10.0f, 1.0f, 0.0f ), 
		glm::vec3( 9.0f, 2.0f, 0.0f ),
		glm::vec3( 8.0f, 3.0f, 0.0f ),

		glm::vec3( 7.0f, 4.0f, 0.0f ), 
		glm::vec3( 6.0f, 5.0f, 0.0f ),
		glm::vec3( 5.0f, 6.0f, 0.0f ),

		glm::vec3( 4.0f, 7.0f, 0.0f ), 
		glm::vec3( 3.0f, 8.0f, 0.0f ),
		glm::vec3( 2.0f, 9.0f, 0.0f ),
	};

	int steps = 10;
	float step = 1.0f / ( float ) steps;

	float t = step, s = step;

	color4f_t color = { 0.0f, 0.3f, 0.5f, 1.0f };

	tbezvert_t first = 
	{
		{ 
			points[ 0 ].x, 
			points[ 0 ].y, 
			points[ 0 ].z 
		},
		{ 
			color.r,
			color.g,
			color.b,
			color.a
		}
	};

	vertices.push_back( first );  

	const int n = 2;
	const int m = 2;

	for ( float u = 0.0f; u <= 1.0f; u += step )
	{
		for ( float v = 0.0f; v <= 1.0f; v += step )
		{
			float z = 0.0f;

			for ( int i = 0; i <= n; ++i )
			{
				float d = ( float ) i * Factorial( ( float ) n - ( float ) i );
				float bi = Factorial( ( float ) n );

				if ( d != 0.0f )
					bi /= d;

				float pu = bi * std::powf( u, ( float ) i ) * std::powf( 1.0f - u, ( float ) n - ( float ) i ); 

				for ( int j = 0; j <= m; ++j )
				{
					float k = ( float ) j * Factorial( ( float ) m - ( float ) j );
					float bj = Factorial( ( float ) m );

					if ( k != 0.0f )
						bj /= k;

					float pv = bj * std::powf( v, ( float ) j ) * std::powf( 1.0f - v, ( float ) m - ( float ) j );
				
					z += bi * pu * bj * pv * points[ i * n + j ].z;
				}
			}

			tbezvert_t vert = 
			{
				{ 
					u, 
					v, 
					z 
				}, 
				{ 
					color.r, 
					color.g, 
					color.b, 
					color.a 
				}
			};

			vertices.push_back( vert );
		}
	}

	tbezvert_t last = 
	{
		{ 
			points[ 8 ].x, 
			points[ 8 ].y, 
			points[ 8 ].z 
		},
		{ 
			color.r,
			color.g,
			color.b,
			color.a
		}
	};

	vertices.push_back( last ); 

    glClearColor( 0.3f, 0.0f, 0.0f, 1.0f );

    camPtr = new InputCamera();
	camPtr->moveStep = 1.0f;

    const float pi = glm::pi< float >();

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

    GL_CHECK( attribPos = glGetAttribLocation( program, "position" ) );
    GL_CHECK( attribCol = glGetAttribLocation( program, "color" ) );

    GL_CHECK( glUseProgram( program ) );
    GL_CHECK( glUniformMatrix4fv( glGetUniformLocation( program, "cameraToClip" ), 1, GL_FALSE, glm::value_ptr( camPtr->ViewData().clipTransform ) ) );

    GL_CHECK( glGenBuffers( 1, &vbo ) );
    GL_CHECK( glBindBuffer( GL_ARRAY_BUFFER, vbo ) );
    GL_CHECK( glBufferData( GL_ARRAY_BUFFER, sizeof( tbezvert_t ) * vertices.size(), &vertices[ 0 ], GL_STATIC_DRAW ) );

    GL_CHECK( glGenVertexArrays( 1, &vao ) );
    GL_CHECK( glBindVertexArray( vao ) );

    // Load vertices
    GL_CHECK( glEnableVertexAttribArray( attribPos ) );
    GL_CHECK( glVertexAttribPointer( attribPos, 3, GL_FLOAT, GL_FALSE, sizeof( tbezvert_t ), ( void* ) offsetof( tbezvert_t, vertex ) ) );

    // Load colors
    GL_CHECK( glEnableVertexAttribArray( attribCol ) );
    GL_CHECK( glVertexAttribPointer( attribCol, 4, GL_FLOAT, GL_FALSE, sizeof( tbezvert_t ), ( void* ) offsetof( tbezvert_t, color ) ) );

    GL_CHECK( glBindVertexArray( 0 ) );

    GL_CHECK( glBindBuffer( GL_ARRAY_BUFFER, 0 ) );
}

const glm::mat4& bezModel = glm::scale( glm::mat4( 1.0f ), glm::vec3( 1.0f ) );

void TBezSurface::Run( void )
{
    camPtr->Update();

    GL_CHECK( glUniformMatrix4fv( glGetUniformLocation( program, "modelToCamera" ), 1, GL_FALSE, glm::value_ptr( camPtr->ViewData().transform * bezModel ) ) );

    GL_CHECK( glBindVertexArray( vao ) );

    GL_CHECK( glBindBuffer( GL_ARRAY_BUFFER, vbo ) );
    GL_CHECK( glDrawArrays( GL_LINE_STRIP, 0, vertices.size() ) );
    GL_CHECK( glBindVertexArray( 0 ) );
}
