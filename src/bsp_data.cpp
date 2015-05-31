#include "bsp_data.h"
#include "glutil.h"

static const float INVERSE_255 = 1.0f / 255.0f;

static glm::u8vec4 BlendColor( const glm::u8vec4& a, const glm::u8vec4& b )
{
	float aaNorm = a[ 3 ] * INVERSE_255;
 	float alpha = aaNorm + b[ 3 ] * INVERSE_255 * ( 1.0f - aaNorm ); 
	float inverseAlpha = 1.0f / alpha;

	glm::vec3 anorm( glm::vec3( a ) * INVERSE_255 );
	glm::vec3 bnorm( glm::vec3( b ) * INVERSE_255 );

	glm::vec3 rgb( ( anorm * aaNorm + bnorm * ( 1.0f - aaNorm ) ) * inverseAlpha );

	glm::u8vec4 ret;

	ret.r = ( uint8_t )( rgb.r * 255.0f );
	ret.g = ( uint8_t )( rgb.g * 255.0f );
	ret.b = ( uint8_t )( rgb.b * 255.0f );
	ret.a = ( uint8_t )( alpha * 255.0f );

	return ret;
}


//-------------------------------------------------------------------------------

bspVertex_t::bspVertex_t( void )
	: bspVertex_t( glm::zero< glm::vec3 >(), glm::zero< glm::vec3 >(), glm::zero< glm::vec2 >(), glm::zero< glm::vec2 >(), glm::zero< glm::u8vec4 >() )
{
}

bspVertex_t::bspVertex_t( const glm::vec3& pos, const glm::vec3& norm, const glm::vec2& surfTexCoords, const glm::vec2& lightmapTexCoords, const glm::u8vec4& color_ )
	: position( pos ),
	  normal( norm ),
	  color( color_ )
{
	texCoords[ 0 ] = surfTexCoords;
	texCoords[ 1 ] = lightmapTexCoords;
}

bspVertex_t::bspVertex_t( const bspVertex_t& v )
	: position( v.position ),
	  normal( v.normal ),
	  color( v.color )
{
	memcpy( texCoords, v.texCoords, sizeof( texCoords ) );
}

bspVertex_t& bspVertex_t::operator=( bspVertex_t v )
{
	position = v.position;
	normal = v.normal;
	color = v.color;

	memcpy( texCoords, v.texCoords, sizeof( texCoords ) );

	return *this;
}

bspVertex_t operator +( const bspVertex_t& a, const bspVertex_t& b )
{
	bspVertex_t vert;
	
	vert.position = a.position + b.position;
	vert.color = BlendColor( a.color, b.color );
	vert.normal = a.normal + b.normal;
	vert.texCoords[ 0 ] = a.texCoords[ 0 ] + b.texCoords[ 0 ];
    vert.texCoords[ 1 ] = a.texCoords[ 1 ] + b.texCoords[ 1 ];

	// TODO: lightmapCoords?

	return vert;
}

bspVertex_t operator -( const bspVertex_t& a, const bspVertex_t& b )
{
	bspVertex_t vert;
	
	vert.position = a.position - b.position;
	vert.color = BlendColor( b.color, a.color );
	vert.normal = a.normal - b.normal;
	vert.texCoords[ 0 ] = a.texCoords[ 0 ] - b.texCoords[ 0 ];
    vert.texCoords[ 1 ] = a.texCoords[ 1 ] - b.texCoords[ 1 ];

	return vert;
}

bspVertex_t operator *( const bspVertex_t& a, float b )
{
	bspVertex_t vert;
	
	vert.position = a.position * b;
	vert.normal = a.normal * b;
	vert.texCoords[ 0 ] = a.texCoords[ 0 ] * b;
    vert.texCoords[ 1 ] = a.texCoords[ 1 ] * b;

	vert.color = a.color;

	return vert;
}

bspVertex_t& operator += ( bspVertex_t& a, const bspVertex_t& b )
{
	a.position += b.position;
	a.normal += b.normal;
	a.texCoords[ 0 ] += b.texCoords[ 0 ];
	a.texCoords[ 1 ] += b.texCoords[ 1 ];

	return a;
}

bool operator == ( const bspVertex_t&a, const bspVertex_t& b )
{
	return a.position == b.position 
		&& a.texCoords[ 0 ] == b.texCoords[ 0 ]
		&& a.texCoords[ 1 ] == b.texCoords[ 1 ]
		&& a.normal == b.normal
		&& a.color == b.color;
}

mapModel_t::mapModel_t( void )
	: deform( false ),
	  vbo( 0 ),
	  subdivLevel( 0 )
{
}

mapModel_t::~mapModel_t( void )
{
	if ( vbo )
	{
		DeleteBufferObject( GL_ARRAY_BUFFER, vbo );
	}
}

