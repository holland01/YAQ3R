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
			GL_CHECK_WITH_NAME( glVertexAttribDivisor( program.attribs.at( "position" ), 0 ), "attribLoadFunctions" ); 
		}
	},
	{
		"normal",
		[]( const Program& program ) -> void
		{
			MapVec3( program.attribs.at( "normal" ), offsetof( bspVertex_t, normal ) );
			GL_CHECK_WITH_NAME( glVertexAttribDivisor( program.attribs.at( "normal" ), 0 ), "attribLoadFunctions" ); 
		}
	},
	{
		"color",
		[]( const Program& program ) -> void
		{
			GL_CHECK_WITH_NAME( glEnableVertexAttribArray( program.attribs.at( "color" ) ), "attribLoadFunctions" ); 
			GL_CHECK_WITH_NAME( glVertexAttribPointer( program.attribs.at( "color" ), 4, GL_UNSIGNED_BYTE, 
				GL_TRUE, sizeof( bspVertex_t ), ( void* ) offsetof( bspVertex_t, color ) ), "attribLoadFunctions" );
			GL_CHECK_WITH_NAME( glVertexAttribDivisor( program.attribs.at( "color" ), 0 ), "attribLoadFunctions" ); 
		}
	},
	{
		"tex0",
		[]( const Program& program ) -> void
		{
			MapVec3( program.attribs.at( "tex0" ), offsetof( bspVertex_t, texCoords[ 0 ] ) );
			GL_CHECK_WITH_NAME( glVertexAttribDivisor( program.attribs.at( "tex0" ), 0 ), "attribLoadFunctions" ); 
		}
	},
	{
		"lightmap",
		[]( const Program& program ) -> void
		{
			MapVec3( program.attribs.at( "lightmap" ), offsetof( bspVertex_t, texCoords[ 1 ] ) );
			GL_CHECK_WITH_NAME( glVertexAttribDivisor( program.attribs.at( "lightmap" ), 0 ), "attribLoadFunctions" ); 
		}
	}
};

GLuint GenSampler( bool mipmap, GLenum wrap )
{
	GLuint sampler;
	GL_CHECK( glGenSamplers( 1, &sampler ) );

	GL_CHECK( glSamplerParameteri( sampler, GL_TEXTURE_MIN_FILTER, mipmap? GL_LINEAR_MIPMAP_LINEAR: GL_LINEAR ) );
	GL_CHECK( glSamplerParameteri( sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR ) );
	GL_CHECK( glSamplerParameteri( sampler, GL_TEXTURE_WRAP_S, wrap ) );
	GL_CHECK( glSamplerParameteri( sampler, GL_TEXTURE_WRAP_T, wrap ) );
	GL_CHECK( glSamplerParameteri( sampler, GL_TEXTURE_WRAP_R, wrap ) );

	GLfloat maxSamples;
	GL_CHECK( glGetFloatv( GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxSamples ) );
	GL_CHECK( glSamplerParameterf( sampler, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxSamples ) );

	return sampler;
}

void BindTexture( GLenum target, GLuint handle, int32_t offset,  
	int32_t sampler, const std::string& uniform, const Program& program )
{
	GL_CHECK( glActiveTexture( GL_TEXTURE0 + offset ) );
	GL_CHECK( glBindTexture( target, handle ) );
	GL_CHECK( glBindSampler( offset, sampler ) );
	program.LoadInt( uniform, offset );
}

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

void ImPrep( const glm::mat4& viewTransform, const glm::mat4& clipTransform )
{
	GL_CHECK( glUseProgram( 0 ) );
	GL_CHECK( glMatrixMode( GL_PROJECTION ) );
	GL_CHECK( glLoadIdentity() );
	GL_CHECK( glLoadMatrixf( glm::value_ptr( clipTransform ) ) );
	GL_CHECK( glMatrixMode( GL_MODELVIEW ) );
	GL_CHECK( glLoadIdentity() );
	GL_CHECK( glLoadMatrixf( glm::value_ptr( viewTransform ) ) );
}

void ImDrawAxes( const float size ) 
{
	std::array< glm::vec3, 6 > axes = 
	{
		glm::vec3( size, 0.0f, 0.0f ), glm::vec3( 1.0f, 0.0f, 0.0f ),
		glm::vec3( 0.0f, size, 0.0f ), glm::vec3( 0.0f, 1.0f, 0.0f ),
		glm::vec3( 0.0f, 0.0f, -size ), glm::vec3( 0.0f, 0.0f, 1.0f )
	};
	
	glBegin( GL_LINES );
	for ( int i = 0; i < 6; i += 2 )
	{
		glColor3fv( glm::value_ptr( axes[ i + 1 ] ) );
		glVertex3f( 0.0f, 0.0f, 0.0f );
		glVertex3fv( glm::value_ptr( axes[ i ] ) ); 
	}
	glEnd();
}

void ImDrawBounds( const AABB& bounds, const glm::vec4& color )
{
	const glm::vec3 center( bounds.Center() );

	const std::array< const glm::vec3, 8 > points = 
	{
		bounds.maxPoint - center, 
		glm::vec3( bounds.maxPoint.x, bounds.maxPoint.y, bounds.minPoint.z ) - center,
		glm::vec3( bounds.maxPoint.x, bounds.minPoint.y, bounds.minPoint.z ) - center, 
		glm::vec3( bounds.maxPoint.x, bounds.minPoint.y, bounds.maxPoint.z ) - center,
		
		bounds.minPoint - center,
		glm::vec3( bounds.minPoint.x, bounds.maxPoint.y, bounds.minPoint.z ) - center,
		glm::vec3( bounds.minPoint.x, bounds.minPoint.y, bounds.minPoint.z ) - center, 
		glm::vec3( bounds.minPoint.x, bounds.minPoint.y, bounds.maxPoint.z ) - center,
	};
	
	GL_CHECK( glMatrixMode( GL_MODELVIEW ) );
	GL_CHECK( glPushMatrix() );
	GL_CHECK( glTranslatef( center.x, center.y, center.z ) );
	
	glBegin( GL_LINE_STRIP );
	glColor4fv( glm::value_ptr( color ) );

	for ( int i = 0; i < 8; ++i )
	{
		glVertex3fv( glm::value_ptr( points[ i ] ) );
	}

	glEnd();
	GL_CHECK( glPopMatrix() );
}

void ImDrawPoint( const glm::vec3& point, const glm::vec4& color, float size )
{
	GL_CHECK( glMatrixMode( GL_MODELVIEW ) );
	GL_CHECK( glPushMatrix() );
	GL_CHECK( glTranslatef( point.x, point.y, point.z ) );
	GL_CHECK( glPushAttrib( GL_POINT_BIT ) );
	GL_CHECK( glPointSize( size ) );

	glBegin( GL_POINTS );
	glColor4fv( glm::value_ptr( color ) );
	glVertex3f( 0.0f, 0.0f, 0.0f );
	glEnd();

	GL_CHECK( glPopAttrib() );
	GL_CHECK( glPopMatrix() );
}

void SetPolygonOffsetState( bool enable, uint32_t polyFlags )
{
	if ( enable )
	{
		if ( polyFlags & GLUTIL_POLYGON_OFFSET_FILL ) GL_CHECK( glEnable( GL_POLYGON_OFFSET_FILL ) );
		if ( polyFlags & GLUTIL_POLYGON_OFFSET_LINE ) GL_CHECK( glEnable( GL_POLYGON_OFFSET_LINE ) );
		if ( polyFlags & GLUTIL_POLYGON_OFFSET_POINT ) GL_CHECK( glEnable( GL_POLYGON_OFFSET_POINT ) );
	}
	else
	{
		if ( polyFlags & GLUTIL_POLYGON_OFFSET_FILL ) GL_CHECK( glDisable( GL_POLYGON_OFFSET_FILL ) );
		if ( polyFlags & GLUTIL_POLYGON_OFFSET_LINE ) GL_CHECK( glDisable( GL_POLYGON_OFFSET_LINE ) );
		if ( polyFlags & GLUTIL_POLYGON_OFFSET_POINT ) GL_CHECK( glDisable( GL_POLYGON_OFFSET_POINT ) );
	}
}

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
	const std::vector< std::string >& uniforms, const std::vector< std::string >& attribs, bool bindTransformsUbo )
    : Program( vertexShader, fragmentShader, bindTransformsUbo? std::vector< std::string >(): attribs )
{
	GenData( uniforms, attribs, bindTransformsUbo );
}

Program::Program( const std::vector< char >& vertexShader, const std::vector< char >& fragmentShader, 
		const std::vector< std::string >& uniforms, const std::vector< std::string >& attribs, bool bindTransformsUbo )
		: Program( std::string( &vertexShader[ 0 ], vertexShader.size() ), 
                std::string( &fragmentShader[ 0 ], fragmentShader.size() ), bindTransformsUbo? std::vector< std::string >(): attribs )
{
    GenData( uniforms, attribs, bindTransformsUbo );
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
	const std::vector< std::string >& attribs, bool bindTransformsUbo )
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

	if ( bindTransformsUbo )
	{
		MapProgramToUBO( program, "Transforms" );
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


