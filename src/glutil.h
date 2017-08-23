#pragma once

#include "common.h"
#include "bsp_data.h"
#include "io.h"
#include <array>
#include <tuple>
#include <unordered_map>
#include "renderer/renderer_local.h"

#define ATTRIB_OFFSET( type, member )( ( void* ) offsetof( type, member ) )

// Extensions
#define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF

#if defined( _DEBUG_USE_GL_ASYNC_CALLBACK ) || defined( _DEBUG_USE_GL_GET_ERR )

#   define GL_CHECK_WITH_NAME( expr, funcname )\
		do\
		{\
			( expr );\
			ExitOnGLError( _LINE_NUM_, #expr, funcname );\
		}\
		while ( 0 )

#   if defined( _DEBUG_USE_GL_ASYNC_CALLBACK )
#       define GL_CHECK( expr )\
			do\
			{\
				( expr );\
				glDebugSetCallInfo( std::string( #expr ), _FUNC_NAME_ );\
			}\
			while ( 0 )
#   elif defined( _DEBUG_USE_GL_GET_ERR )
#       define GL_CHECK( expr )\
			do\
			{\
				( expr );\
				ExitOnGLError( _LINE_NUM_, #expr, _FUNC_NAME_ );\
			}\
			while ( 0 )
#   endif
#else
#	define GL_CHECK( expr ) ( expr )
#   define GL_CHECK_WITH_NAME( expr, funcname ) ( expr )
#endif // _DEBUG_USE_GL_ASYNC_CALLBACK

#ifdef GLES
#	define glClearDepth glClearDepthf
#endif

void GPrintBadProgram( void );
bool GHasBadProgram( void );
void GStateCheckReport( void );

enum
{
	GLUTIL_POLYGON_OFFSET_FILL = 1 << 0,
	GLUTIL_POLYGON_OFFSET_LINE = 1 << 1,
	GLUTIL_POLYGON_OFFSET_POINT = 1 << 2,
	GLUTIL_POLYGON_OFFSET_ALL = 0x7,

	GLUTIL_LAYOUT_POSITION = 1 << 0,
	GLUTIL_LAYOUT_COLOR = 1 << 1,
	GLUTIL_LAYOUT_TEX0 = 1 << 2,
	GLUTIL_LAYOUT_LIGHTMAP = 1 << 3,
	GLUTIL_LAYOUT_NORMAL = 1 << 4,
	GLUTIL_LAYOUT_ALL = 0x1F,

	GLUTIL_NUM_ATTRIBS_MAX = 5
};

class GLConfig
{
public:
	// must match the same number used in main.frag
	static const int32_t MAX_MIP_LEVELS = 16;
};

class Program;
class AABB;

static INLINE void MapVec3( int location, size_t offset )
{
	GL_CHECK( glEnableVertexAttribArray( location ) );
	GL_CHECK( glVertexAttribPointer( location, 3, GL_FLOAT, GL_FALSE,
				sizeof( bspVertex_t ), ( void* ) offset ) );
}

static INLINE void MapUniforms( glHandleMap_t& unifMap, GLuint programID,
		const std::vector< std::string >& uniforms )
{
	for ( const std::string& title: uniforms )
	{
		GLint uniform;
		GL_CHECK( uniform = glGetUniformLocation( programID, title.c_str() ) );
		unifMap.insert( glHandleMapEntry_t( title, uniform ) );
	}
}

static INLINE GLuint MakeGenericBufferObject( void )
{
	GLuint obj = 0;
	GL_CHECK( glGenBuffers( 1, &obj ) );

	if ( !obj )
	{
		MLOG_ERROR( "call to glGenBuffers returned 0" );
	}

	return obj;
}

template < typename T >
static INLINE GLuint GenBufferObject( GLenum target,
	const std::vector< T >& data, GLenum usage )
{
	GLuint obj = MakeGenericBufferObject();

	GL_CHECK( glBindBuffer( target, obj ) );
	GL_CHECK( glBufferData( target, data.size() * sizeof( T ),
		&data[ 0 ], usage ) );
	GL_CHECK( glBindBuffer( target, 0 ) );

	return obj;
}

template < typename T >
static INLINE void UpdateBufferObject( GLenum target, GLuint obj, GLuint offset,
		const std::vector< T >& data, bool bindUnbind )
{
	if ( bindUnbind )
	{
		GL_CHECK( glBindBuffer( target, obj ) );
	}

	GL_CHECK( glBufferSubData( target, offset * sizeof( T ),
		data.size() * sizeof( T ), &data[ 0 ] ) );

	if ( bindUnbind )
	{
		GL_CHECK( glBindBuffer( target, 0 ) );
	}
}

static INLINE void DeleteBufferObject( GLenum target, GLuint obj )
{
	if ( obj )
	{
		// Unbind to prevent driver from lazy deletion
		GL_CHECK( glBindBuffer( target, 0 ) );
		GL_CHECK( glDeleteBuffers( 1, &obj ) );
	}
}

static INLINE void DrawElementBuffer( GLuint ibo, size_t numIndices )
{
	GL_CHECK( glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibo ) );
	GL_CHECK( glDrawElements( GL_TRIANGLES, numIndices, GL_UNSIGNED_INT,
		nullptr ) );
	GL_CHECK( glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 ) );
}

static INLINE void SetTex2DMinMagFilters( GLenum min, GLenum mag )
{
	GL_CHECK( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		min ) );
	GL_CHECK( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
		mag ) );
}

static INLINE GLuint MakeTex2DRGBA( uint8_t* data, GLsizei width, GLsizei height )
{
	GLuint texObj;
	GL_CHECK( glGenTextures( 1, &texObj ) );
	GL_CHECK( glBindTexture( GL_TEXTURE_2D, texObj ) );

	GL_CHECK( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
		GL_CLAMP_TO_EDGE ) );
	GL_CHECK( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
		GL_CLAMP_TO_EDGE ) );

	SetTex2DMinMagFilters( GL_LINEAR, GL_LINEAR );

	GL_CHECK(
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			G_INTERNAL_RGBA_FORMAT,
			width, height,
			0,
			G_RGBA_FORMAT,
			GL_UNSIGNED_BYTE,
			data
		)
	);

	GL_CHECK( glBindTexture( GL_TEXTURE_2D, 0 ) );

	return texObj;
}

struct attribProfile_t
{
	std::string id;

	GLuint location;
	GLint tupleSize;
	GLenum apiType;
	GLboolean normalized = GL_FALSE;
	GLsizei stride;
	uintptr_t offset;
};

class Program
{
private:
	GLuint program;

#define DECL_SHADER_STORE( Type, name ) \
		using t_##name = std::unordered_map< GLint, Type >;\
		mutable t_##name name


	DECL_SHADER_STORE( glm::mat4, mat4s );
	DECL_SHADER_STORE( glm::mat3, mat3s );
	DECL_SHADER_STORE( glm::mat2, mat2s );

	DECL_SHADER_STORE( glm::vec4, vec4s );
	DECL_SHADER_STORE( glm::vec3, vec3s );
	DECL_SHADER_STORE( glm::vec2, vec2s );

	DECL_SHADER_STORE( std::vector< glm::vec2 >, vec2Array );
	DECL_SHADER_STORE( std::vector< glm::vec3 >, vec3Array );
	DECL_SHADER_STORE( std::vector< glm::vec4 >, vec4Array );

	DECL_SHADER_STORE( float, floats );
	DECL_SHADER_STORE( int, ints );

#undef DECL_SHADER_STORE

	void GenData( const std::vector< std::string >& uniforms,
		const std::vector< std::string >& attribs );

	std::vector< attribProfile_t > altAttribProfiles;

public:

	programDataMap_t uniforms;
	programDataMap_t attribs;

	const shaderStage_t* stage;

	// Cleared on each invocation of LoadAttribLayout
	std::vector< std::string > disableAttribs;

#ifdef DEBUG
 	std::string vertexSource;
	std::string fragmentSource;
#endif

	Program( const std::string& vertexShader,
			 const std::string& fragmentShader,
			 const std::vector< std::string >& bindAttribs =
			std::vector< std::string >() );

	Program( const std::string& vertexShader,
			const std::string& fragmentShader,
		const std::vector< std::string >& uniforms,
		const std::vector< std::string >& attribs );

	Program( const std::vector< char >& vertexShader,
		const std::vector< char >& fragmentShader,
		const std::vector< std::string >& uniforms,
		const std::vector< std::string >& attribs );

	Program( const Program& copy );

	~Program( void );

	GLuint GetHandle( void ) const { return program; }

	void AddUnif( const std::string& name );

	void AddAttrib( const std::string& name );

	void AddAltAttribProfile( const attribProfile_t& profile );

	void LoadDefaultAttribProfiles( void ) const;

	void DisableDefaultAttribProfiles( void ) const;

	void LoadAltAttribProfiles( void ) const;

	void DisableAltAttribProfiles( void ) const;

	void LoadMat4( const std::string& name, const glm::mat4& t ) const;

	void LoadMat2( const std::string& name, const glm::mat2& t ) const;
	void LoadMat2( const std::string& name, const float* t ) const;

	void LoadVec2( const std::string& name, const glm::vec2& v ) const;
	void LoadVec2( const std::string& name, const float* v ) const;

	void LoadVec2Array( const std::string& name,
		const float* v, int32_t num ) const;

	void LoadVec3( const std::string& name, const glm::vec3& v ) const;

	void LoadVec3Array( const std::string& name, const float* v,
		int32_t num ) const;

	void LoadVec4( const std::string& name, const glm::vec4& v ) const;
	void LoadVec4( const std::string& name, const float* v ) const;

	void LoadVec4Array( const std::string& name, const float* v,
		int32_t num ) const;

	void LoadInt( const std::string& name, int v ) const;

	void LoadFloat( const std::string& name, float v ) const;

	void Bind( void ) const;
	void Release( void ) const;

	static std::vector< std::string > ArrayLocationNames(
		const std::string& name, int32_t length );

	std::string GetInfoString( void ) const;
};

INLINE void Program::AddUnif( const std::string& name )
{
	GL_CHECK( uniforms[ name ] = glGetUniformLocation(
		program, name.c_str() ) );
}

INLINE void Program::AddAttrib( const std::string& name )
{
	GL_CHECK( attribs[ name ] = glGetAttribLocation( program,
		name.c_str() ) );
}

INLINE void Program::AddAltAttribProfile( const attribProfile_t& profile )
{
	altAttribProfiles.push_back( profile );
}

template < typename vecType_t, uint32_t tupleSize >
static INLINE typename std::vector< vecType_t > MakeVectorArray(
	const float* v, int32_t num )
{
	uint32_t cnum = num;

	std::vector< vecType_t > buf;
	buf.resize( cnum );

	// A memcpy is probably not very trustworthy, here...
	for ( uint32_t i = 0; i < cnum; ++i )
	{
		for ( uint32_t k = 0; k < tupleSize; ++k )
			buf[ i ][ k ] = v[ i * tupleSize + k ];
	}

	return std::move( buf );
}

INLINE void Program::LoadMat4( const std::string& name,
	const glm::mat4& t ) const
{
	mat4s.insert( t_mat4s::value_type( uniforms.at( name ), t ) );
}

INLINE void Program::LoadMat2( const std::string& name,
	const glm::mat2& t ) const
{
	mat2s.insert( t_mat2s::value_type( uniforms.at( name ), t ) );
}

INLINE void Program::LoadMat2( const std::string& name,
	const float* t ) const
{
	glm::mat2 m( t[ 0 ], t[ 1 ],
				 t[ 2 ], t[ 3 ] );

	mat2s.insert( t_mat2s::value_type( uniforms.at( name ), m ) );
}

INLINE void Program::LoadVec2( const std::string& name,
	const glm::vec2& v ) const
{
	vec2s.insert( t_vec2s::value_type( uniforms.at( name ), v ) );;
}

INLINE void Program::LoadVec2( const std::string& name,
	const float* v ) const
{
	glm::vec2 v0( v[ 0 ], v[ 1 ] );

	vec2s.insert( t_vec2s::value_type( uniforms.at( name ), v0 ) );
}

INLINE void Program::LoadVec2Array( const std::string& name, const float* v,
	int32_t num ) const
{
	vec2Array.insert( t_vec2Array::value_type( uniforms.at( name ),
		MakeVectorArray< glm::vec2, 2 >( v, num ) ) );
}

INLINE void Program::LoadVec3( const std::string& name,
	const glm::vec3& v ) const
{
	vec3s.insert( t_vec3s::value_type( uniforms.at( name ), v ) );
}

INLINE void Program::LoadVec3Array( const std::string& name, const float* v,
	int32_t num ) const
{
	vec3Array.insert( t_vec3Array::value_type( uniforms.at( name ),
		MakeVectorArray< glm::vec3, 3 >( v, num ) ) );
}

INLINE void Program::LoadVec4( const std::string& name,
	const glm::vec4& v ) const
{
	vec4s.insert( t_vec4s::value_type( uniforms.at( name ), v ) );
}

INLINE void Program::LoadVec4( const std::string& name,
	const float* v ) const
{
	glm::vec4 v0( v[ 0 ], v[ 1 ], v[ 2 ], v[ 3 ] );

	vec4s.insert( t_vec4s::value_type( uniforms.at( name ), v0 ) );
}

INLINE void Program::LoadVec4Array( const std::string& name, const float* v,
	int32_t num ) const
{
	vec4Array.insert( t_vec4Array::value_type( uniforms.at( name ),
		MakeVectorArray< glm::vec4, 4 >( v, num ) ) );
}

INLINE void Program::LoadInt( const std::string& name, int v ) const
{
	ints.insert( t_ints::value_type( uniforms.at( name ), v ) );
}

INLINE void Program::LoadFloat( const std::string& name, float f ) const
{
	floats.insert( t_floats::value_type( uniforms.at( name ), f ) );
}

//-------------------------------------------------------------------------------------------------
struct loadBlend_t
{
	GLenum prevSrcFactor, prevDstFactor;

	loadBlend_t( GLenum srcFactor, GLenum dstFactor );
   ~loadBlend_t( void );
};

struct viewportStash_t
{
	std::array< GLint, 4 > original;

	viewportStash_t( GLint originX, GLint originY, GLint width, GLint height )
	{
		GL_CHECK( glGetIntegerv( GL_VIEWPORT, &original[ 0 ] ) );
		GL_CHECK( glViewport( originX, originY, width, height ) );
	}

	~viewportStash_t( void )
	{
		GL_CHECK( glViewport( original[ 0 ], original[ 1 ],
			original[ 2 ], original[ 3 ] ) );
	}
};
//-------------------------------------------------------------------------------------------------

struct immDebugVertex_t
{
	glm::vec3 position;
	glm::u8vec4 color;
};

class ImmDebugDraw
{
	GLuint vbo;

	size_t previousSize;

	bool isset;

	immDebugVertex_t thisVertex;

	std::vector< immDebugVertex_t > vertices;

	std::unordered_map< std::string, std::unique_ptr< Program > > shaderPrograms;

	void Finalize( bool setIsset = true );

public:
	ImmDebugDraw( void );

	~ImmDebugDraw( void );

	const Program* GetProgram( const std::string& which = "default" ) const { return shaderPrograms.at( which ).get(); }

	void Begin( void );

	void Position( const glm::vec3& position );

	void Color( const glm::u8vec4& color );

	void End( GLenum mode, const glm::mat4& projection,
		const glm::mat4& modelView );
};
