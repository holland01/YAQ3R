#pragma once

#include "common.h"

static const float INVERSE_255 = 1.0f / 255.0f;

static INLINE glm::u8vec4 BlendColor( const glm::u8vec4& a, const glm::u8vec4& b )
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

template < typename texCoord_t >
struct vertex_t
{
	glm::vec3	position;
    texCoord_t	texCoords[ 2 ]; // 0 => surface, 1 => lightmap
    glm::vec3	normal;

    glm::u8vec4 color;

	vertex_t( void )
		: vertex_t< texCoord_t >( 
			glm::zero< glm::vec3  >(), 
			glm::zero< glm::vec3  >(), 
			glm::zero< texCoord_t >(), 
			glm::zero< texCoord_t >(), 
			glm::zero< glm::u8vec4 >() )
	{
	}
	
	vertex_t( 
		const glm::vec3& pos, 
		const glm::vec3& norm, 
		const texCoord_t& surfTexCoords, 
		const texCoord_t& lightmapTexCoords, 
		const glm::u8vec4& color_ )
		: position( pos ),
		  normal( norm ),
		  color( color_ )
	{
		texCoords[ 0 ] = surfTexCoords;
		texCoords[ 1 ] = lightmapTexCoords;
	}

	vertex_t( const vertex_t& v )
		: position( v.position ),
		  normal( v.normal ),
		  color( v.color )
	{
		memcpy( texCoords, v.texCoords, sizeof( texCoords ) );
	}

	vertex_t< texCoord_t >& operator=( vertex_t< texCoord_t > v )
	{
		position = v.position;
		normal = v.normal;
		color = v.color;

		memcpy( texCoords, v.texCoords, sizeof( texCoords ) );

		return *this;
	}

};

template < typename texCoord_t >
INLINE vertex_t< texCoord_t > operator +( const vertex_t< texCoord_t >& a, const vertex_t< texCoord_t >& b )
{
	vertex_t< texCoord_t > vert;
	
	vert.position = a.position + b.position;
	vert.color = BlendColor( a.color, b.color );
	vert.normal = a.normal + b.normal;
	vert.texCoords[ 0 ] = a.texCoords[ 0 ] + b.texCoords[ 0 ];
	vert.texCoords[ 1 ] = a.texCoords[ 1 ] + b.texCoords[ 1 ];

	return vert;
}

template < typename texCoord_t >
vertex_t< texCoord_t > operator -( const vertex_t< texCoord_t >& a, const vertex_t< texCoord_t >& b )
{
	vertex_t< texCoord_t > vert;
	
	vert.position = a.position - b.position;
	vert.color = BlendColor( b.color, a.color );
	vert.normal = a.normal - b.normal;
	vert.texCoords[ 0 ] = a.texCoords[ 0 ] - b.texCoords[ 0 ];
	vert.texCoords[ 1 ] = a.texCoords[ 1 ] - b.texCoords[ 1 ];

	return vert;
}

template < typename texCoord_t >
vertex_t< texCoord_t > operator *( const vertex_t< texCoord_t >& a, float b )
{
	vertex_t< texCoord_t > vert;
	
	vert.position = a.position * b;
	vert.normal = a.normal * b;
	vert.texCoords[ 0 ] = a.texCoords[ 0 ] * b;
	vert.texCoords[ 1 ] = a.texCoords[ 1 ] * b;

	vert.color = a.color;

	return vert;
}

template < typename texCoord_t >
vertex_t< texCoord_t >& operator += ( vertex_t< texCoord_t >& a, const vertex_t< texCoord_t >& b )
{
	a.position += b.position;
	a.normal += b.normal;
	a.texCoords[ 0 ] += b.texCoords[ 0 ];
	a.texCoords[ 1 ] += b.texCoords[ 1 ];

	return a;
}

template < typename texCoord_t >
bool operator == ( const vertex_t< texCoord_t >&a, const vertex_t< texCoord_t >& b )
{
	return a.position == b.position 
		&& a.texCoords[ 0 ] == b.texCoords[ 0 ]
		&& a.texCoords[ 1 ] == b.texCoords[ 1 ]
		&& a.normal == b.normal
		&& a.color == b.color;
}

using bspVertex_t = vertex_t< glm::vec2 >;
using drawVertex_t = vertex_t< glm::vec3 >;

//-------------------------------------------------------------------------------



