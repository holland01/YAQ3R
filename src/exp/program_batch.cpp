#include "program_batch.h"

Pipeline::Pipeline( void )
{
}

Pipeline::~Pipeline( void )
{
    glUseProgram( 0 );
    current = NULL;

    glDeleteBuffers( UBO_COUNT, ubos );

    auto end = programs.end();

    for ( auto i = programs.begin(); i != end; ++i )
    {
        Program* p = i->second;

        glDeleteProgram( p->handle );

        delete p;
    }
}

void Pipeline::Alloc( void )
{
    glGenBuffers( UBO_COUNT, ubos );

    glBindBuffer( GL_UNIFORM_BUFFER, ubos[ UBO_GM4 ] );
    glBufferData( GL_UNIFORM_BUFFER, UBO_GM4_SIZE, NULL, GL_DYNAMIC_DRAW );
    glBindBuffer( GL_UNIFORM_BUFFER, 0 );

    glBindBuffer( GL_UNIFORM_BUFFER, ubos[ UBO_GM3 ] );
    glBufferData( GL_UNIFORM_BUFFER, UBO_GM3_SIZE, NULL, GL_DYNAMIC_DRAW );
    glBindBuffer( GL_UNIFORM_BUFFER, 0 );

    glBindBufferBase( GL_UNIFORM_BUFFER, UBO_GM4, ubos[ UBO_GM4 ] );
    glBindBufferBase( GL_UNIFORM_BUFFER, UBO_GM3, ubos[ UBO_GM3 ] );
}

void Pipeline::AddProgram( const std::string& key, GLuint program, const std::vector< std::string >& attribs, const std::vector< std::string >& uniforms )
{
    Program* p = new Program;

    glUniformBlockBinding( program, glGetUniformBlockIndex( program, "GlobalMat4" ), UBO_GM4 );
    glUniformBlockBinding( program, glGetUniformBlockIndex( program, "GlobalMat3" ), UBO_GM3 );

    int end = ( int ) attribs.size();

    for ( int i = 0; i < end; ++i )
    {
        p->attribs[ attribs[ i ] ] = glGetAttribLocation( program, attribs[ i ].c_str() );
    }

    end = ( int ) uniforms.size();

    for ( int i = 0; i < end; ++i )
    {
        p->uniforms[ uniforms[ i ] ] =  glGetUniformLocation( program, uniforms[ i ].c_str() );
    }

    p->handle = program;

    programs.insert( std::map< std::string, Program* >::value_type( key, p ) );
}
