#include "glutil.h"
#include "q3bsp.h"
#include "shader.h"
#include "aabb.h"
#include "renderer/texture.h"
#include <algorithm>

static std::map< std::string, std::function< void( const Program& program ) > > attribLoadFunctions = 
{
	{
		"position",
		[]( const Program& program ) -> void
		{
			MapVec3( program.attribs.at( "position" ), offsetof( bspVertex_t, position ) );
		}
	},
	{
		"normal",
		[]( const Program& program ) -> void
		{
			MapVec3( program.attribs.at( "normal" ), offsetof( bspVertex_t, normal ) );
		}
	},
	{
		"color",
		[]( const Program& program ) -> void
		{
			GL_CHECK_WITH_NAME( glEnableVertexAttribArray( program.attribs.at( "color" ) ), "attribLoadFunctions" ); 
			GL_CHECK_WITH_NAME( glVertexAttribPointer( program.attribs.at( "color" ), 4, GL_UNSIGNED_BYTE, 
				GL_TRUE, sizeof( bspVertex_t ), ( void* ) offsetof( bspVertex_t, color ) ), "attribLoadFunctions" );
		}
	},
	{
		"tex0",
		[]( const Program& program ) -> void
		{
			MapVec3( program.attribs.at( "tex0" ), offsetof( bspVertex_t, texCoords[ 0 ] ) );
		}
	},
	{
		"lightmap",
		[]( const Program& program ) -> void
		{
			MapVec3( program.attribs.at( "lightmap" ), offsetof( bspVertex_t, texCoords[ 1 ] ) );
		}
	}
};

/*
static INLINE void SetPixel( byte* dest, const byte* src, int width, int height, int bpp, int srcX, int srcY, int destX, int destY )
{
	int destOfs = ( width * destY + destX ) * bpp;
	int srcOfs = ( width * srcY + srcX ) * bpp;

	for ( int k = 0; k < bpp; ++k )
		dest[ destOfs + k ] = src[ srcOfs + k ];
}

static INLINE void FlipBytes( byte* out, const byte* src, int width, int height, int bpp )
{
	for ( int y = 0; y < height; ++y )
	{
		for ( int x = 0; x < width; ++x )
		{
			SetPixel( out, src, width, height, bpp, x, height - y - 1, x, y );
		}
	}
}
*/

//-------------------------------------------------------------------------------------------------

static INLINE void DisableAllAttribs( void )
{
    for ( int i = 0; i < 5; ++i )
        GL_CHECK( glDisableVertexAttribArray( i ) );
}

Program::Program( const std::string& vertexShader, const std::string& fragmentShader, const std::vector< std::string >& bindAttribs )
	: program( 0 )
{
	GLuint shaders[] = 
	{
		CompileShaderSource( vertexShader.c_str(), vertexShader.size(), GL_VERTEX_SHADER ),
		CompileShaderSource( fragmentShader.c_str(), fragmentShader.size(), GL_FRAGMENT_SHADER )
	};

    program = LinkProgram( shaders, 2, bindAttribs );
}

Program::Program( const std::string& vertexShader, const std::string& fragmentShader, 
	const std::vector< std::string >& uniforms, const std::vector< std::string >& attribs )
	: Program( vertexShader, fragmentShader, attribs )
{
	GenData( uniforms, attribs );
}

Program::Program( const std::vector< char >& vertexShader, const std::vector< char >& fragmentShader,
		const std::vector< std::string >& uniforms, const std::vector< std::string >& attribs )
		: Program( std::string( &vertexShader[ 0 ], vertexShader.size() ), 
				std::string( &fragmentShader[ 0 ], fragmentShader.size() ), attribs )
{
	GenData( uniforms, attribs );
}

Program::Program( const Program& copy )
	: program( copy.program ),
	  uniforms( copy.uniforms ),
	  attribs( copy.attribs )
{
}

Program::~Program( void )
{
	Release();
	GL_CHECK( glDeleteProgram( program ) );
}

void Program::GenData( const std::vector< std::string >& uniforms, 
	const std::vector< std::string >& attribs )
{
	uint32_t max = glm::max( attribs.size(), uniforms.size() );
	for ( uint32_t i = 0; i < max; ++i )
	{
		if ( i < attribs.size() )
		{
			AddAttrib( attribs[ i ] );
		}

		if ( i < uniforms.size() )
		{
			AddUnif( uniforms[ i ] );
		}
	}
}

void Program::LoadDefaultAttribProfiles( void ) const
{
    DisableAllAttribs();

    for ( const auto& attrib: attribs )
	{
		if ( attrib.second != -1 )
		{

            auto it = std::find( disableAttribs.begin(), disableAttribs.end(), attrib.first );

            if ( it != disableAttribs.end() )
            {
                GL_CHECK( glDisableVertexAttribArray( attrib.second ) );
                continue;
            }
			
			attribLoadFunctions[ attrib.first ]( *this ); 
		}
	}
}

void Program::LoadAltAttribProfiles( void ) const
{
    DisableAllAttribs();

    for ( const attribProfile_t& profile: altAttribProfiles )
    {
        GL_CHECK( glEnableVertexAttribArray( profile.location ) );
        GL_CHECK( glVertexAttribPointer( profile.location, profile.tupleSize, profile.apiType,
                    profile.normalized, profile.stride, ( const GLvoid* )profile.offset ) );

    }
}

std::vector< std::string > Program::ArrayLocationNames( const std::string& name, int32_t length )
{
	std::vector< std::string > names;
	names.resize( length );

	for ( int32_t i = 0; i < length; ++i )
	{
		names[ i ] = name + "[" + std::to_string( i ) + "]";
	}
	return names;
}

#ifdef GLES

#define __LOAD_VEC( f, name ) for ( const auto& v: ( name ) ) GL_CHECK( ( f )( v.first, 1, glm::value_ptr( v.second ) ) )
#define __LOAD_MAT( f, name ) for ( const auto& m: ( name ) ) GL_CHECK( ( f )( m.first, 1, GL_FALSE, glm::value_ptr( m.second ) ) )
#define __LOAD_VEC_ARRAY( f, name ) for ( const auto& v: ( name ) ) GL_CHECK( ( f )( v.first, v.second.size(), &v.second[ 0 ][ 0 ] ) )
#define __LOAD_SCALAR( f, name ) for ( auto i: ( name ) ) GL_CHECK( ( f )( i.first, i.second ) )

void Program::Bind( void ) const
{
    GL_CHECK( glUseProgram( program ) );

    __LOAD_VEC( glUniform2fv, vec2s );
    __LOAD_VEC( glUniform3fv, vec3s );
    __LOAD_VEC( glUniform4fv, vec4s );

    __LOAD_MAT( glUniformMatrix2fv, mat2s );
    __LOAD_MAT( glUniformMatrix3fv, mat3s );
    __LOAD_MAT( glUniformMatrix4fv, mat4s );

    __LOAD_VEC_ARRAY( glUniform2fv, vec2Array );
    __LOAD_VEC_ARRAY( glUniform3fv, vec3Array );
    __LOAD_VEC_ARRAY( glUniform4fv, vec4Array );

    __LOAD_SCALAR( glUniform1i, ints );
    __LOAD_SCALAR( glUniform1f, floats );
}

#undef __LOAD_VEC
#undef __LOAD_MAT
#undef __LOAD_VEC_ARRAY
#undef __LOAD_SCALAR

void Program::Release( void ) const
{
    GL_CHECK( glUseProgram( 0 ) );

    vec2s.clear();
    vec3s.clear();
    vec4s.clear();

    mat2s.clear();
    mat3s.clear();
    mat4s.clear();

    vec2Array.clear();
    vec3Array.clear();
    vec4Array.clear();

    ints.clear();
    floats.clear();
}


#endif // GLES

//-------------------------------------------------------------------------------------------------

loadBlend_t::loadBlend_t( GLenum srcFactor, GLenum dstFactor )
{
	GL_CHECK( glGetIntegerv( GL_BLEND_SRC_RGB, ( GLint* ) &prevSrcFactor ) );
	GL_CHECK( glGetIntegerv( GL_BLEND_DST_RGB, ( GLint* ) &prevDstFactor ) );

	GL_CHECK( glBlendFunc( srcFactor, dstFactor ) );
}

loadBlend_t::~loadBlend_t( void )
{
	GL_CHECK( glBlendFunc( prevSrcFactor, prevDstFactor ) );
}

//-------------------------------------------------------------------------------------------------


