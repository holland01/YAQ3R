#pragma once

#include "../common.h"

enum
{
    GM4_MODELVIEW = 0,
    GM4_PROJECTION = 1,

    GM3_MODELVIEW_NORM = 0,

    UBO_COUNT = 2,
    UBO_GM4 = 0,
    UBO_GM3 = 1,
    UBO_GM4_SIZE = sizeof( glm::mat4 ) * 2,
    UBO_GM3_SIZE = sizeof( glm::mat3 )
};

struct Program
{
    GLuint                          handle;

    std::map< std::string, GLint >  attribs;
    std::map< std::string, GLint >  uniforms;
};

class Pipeline
{
    GLuint                              ubos[ UBO_COUNT ];

    std::map< std::string, Program* >   programs;

    Program*                            current;

public:

    Pipeline( void );
    ~Pipeline( void );

    void Alloc( void );

    void UseProgram( const std::string& key );
    void ReleaseProgram( void );

    Program* GetProgram( const std::string& key );

    GLint Attrib( const std::string& attrib );
    GLint Uniform( const std::string& uniform );

    void AddProgram( const std::string& key, GLuint program, const std::vector< std::string >& attribs, const std::vector< std::string >& uniforms );

    void SetGlobalMat( int index, const glm::mat4& m );
    void SetGlobalMat( int index, const glm::mat3& m );

    void SetUniformVec( const std::string& uniform, const glm::vec4& v );
    void SetUniformVec( const std::string& uniform, const glm::vec3& v );

    void SetUniformScalar( const std::string& uniform, float s );
    void SetUniformScalar( const std::string& uniform, int s );
};

INLINE Program* Pipeline::GetProgram( const std::string& key )
{
    return programs[ key ];
}

INLINE GLint Pipeline::Attrib( const std::string& attrib )
{
    return current->attribs[ attrib ];
}

INLINE GLint Pipeline::Uniform( const std::string& uniform )
{
    return current->attribs[ uniform ];
}

INLINE void Pipeline::UseProgram( const std::string& key )
{
    current = programs[ key ];
    glUseProgram( current->handle );
}

INLINE void Pipeline::ReleaseProgram( void )
{
    current = NULL;
    glUseProgram( 0 );
}

INLINE void Pipeline::SetGlobalMat( int index, const glm::mat4& m )
{
    glBindBuffer( GL_UNIFORM_BUFFER, ubos[ UBO_GM4 ] );
    glBufferSubData( GL_UNIFORM_BUFFER, index * sizeof( glm::mat4 ), sizeof( glm::mat4 ), ( void* ) glm::value_ptr( m ) );
    glBindBuffer( GL_UNIFORM_BUFFER, 0 );
}

INLINE void Pipeline::SetGlobalMat( int index, const glm::mat3& m )
{
    glBindBuffer( GL_UNIFORM_BUFFER, ubos[ UBO_GM3 ] );
    glBufferSubData( GL_UNIFORM_BUFFER, index * sizeof( glm::mat3 ), sizeof( glm::mat3 ), ( void* ) glm::value_ptr( m ) );
    glBindBuffer( GL_UNIFORM_BUFFER, 0 );
}

INLINE void Pipeline::SetUniformVec( const std::string& uniform, const glm::vec4& v )
{
    glUniform4f( current->uniforms[ uniform ], v.x, v.y, v.z, v.w );
}

INLINE void Pipeline::SetUniformVec( const std::string& uniform, const glm::vec3& v )
{
     glUniform3f( current->uniforms[ uniform ], v.x, v.y, v.z );
}

INLINE void Pipeline::SetUniformScalar( const std::string& uniform, float s )
{
    glUniform1f( current->uniforms[ uniform ], s );
}

INLINE void Pipeline::SetUniformScalar( const std::string& uniform, int s )
{
    glUniform1i( current->uniforms[ uniform ], s );
}
