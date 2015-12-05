#pragma once

#include "common.h"
#include <array>

static const float INVERSE_255 = 1.0f / 255.0f;

//-------------------------------------------------------------------------------------------------------------
// vertex_t: generic vertex type for arbitrary use cases
//-------------------------------------------------------------------------------------------------------------

#define VERTEX_TEMPLATE_DECL template < typename position_type, typename texcoord_type, \
    typename normal_type, typename color_type, int32_t positionTupleSize, \
    int32_t texCoordTupleSize, int32_t normalTupleSize, int32_t colorTupleSize, \
    int32_t numTexCoords >

#define VERTEX_TEMPLATE_TYPE vertex_t< position_type, texcoord_type, \
    normal_type, color_type, positionTupleSize, texCoordTupleSize, \
    normalTupleSize, colorTupleSize, numTexCoords >

VERTEX_TEMPLATE_DECL
struct vertex_t
{ 
    using vertex_type = vertex_t< position_type, texcoord_type, normal_type, color_type, positionTupleSize,
       texCoordTupleSize, normalTupleSize, colorTupleSize, numTexCoords >;

    using position_t = position_type[ positionTupleSize ];
    using texcoord_t = texcoord_type[ texCoordTupleSize ];
    using normal_t = normal_type[ normalTupleSize ];
    using color_t = normal_type[ positionTupleSize ];

    position_t   position;
    texcoord_t   texCoords[ numTexCoords ];
    normal_t     normal;
    color_t      color;

	vertex_t( void )
	{
        memset( position, 0, sizeof( position ) );
        memset( texCoords, 0, sizeof( texCoords[ 0 ] ) * 2 );
        memset( normal, 0, sizeof( normal ) );
        memset( color, 0, sizeof( color ) );
	}

    void BlendColor( color_t& outC, const VERTEX_TEMPLATE_TYPE& b )
    {
        for ( int32_t i = 0; i < colorTupleSize; ++i )
        {
            float x = ( float )color[ i ] * INVERSE_255;
            float y = ( float )b.color[ i ] * INVERSE_255;

            outC[ i ] = uint8_t( ( x * y ) * 255.0f );
        }
    }
};

VERTEX_TEMPLATE_DECL
INLINE VERTEX_TEMPLATE_TYPE operator +( const VERTEX_TEMPLATE_TYPE& a, const VERTEX_TEMPLATE_TYPE& b )
{
    VERTEX_TEMPLATE_TYPE vert;
	
    for ( int32_t i = 0; i < positionTupleSize; ++i )
        vert.position[ i ] = a.position[ i ] + b.position[ i ];

    a.BlendColor( vert.color, b );

    for ( int32_t i = 0; i < normalTupleSize; ++i )
        vert.normal[ i ] = a.normal[ i ] + b.normal[ i ];

    for ( int32_t i = 0; i < numTexCoords; ++i )
        for ( int32_t j = 0; j < texCoordTupleSize; ++j )
            vert.texCoords[ i ][ j ] = a.texCoords[ i ][ j ] + b.texCoords[ i ][ j ];

	return vert;
}

VERTEX_TEMPLATE_DECL
INLINE VERTEX_TEMPLATE_TYPE operator -( const VERTEX_TEMPLATE_TYPE& a, const VERTEX_TEMPLATE_TYPE& b )
{
    VERTEX_TEMPLATE_TYPE vert;

    for ( int32_t i = 0; i < positionTupleSize; ++i )
        vert.position[ i ] = a.position[ i ] - b.position[ i ];

    a.BlendColor( vert.color, b );

    for ( int32_t i = 0; i < normalTupleSize; ++i )
        vert.normal[ i ] = a.normal[ i ] - b.normal[ i ];

    for ( int32_t i = 0; i < numTexCoords; ++i )
        for ( int32_t j = 0; j < texCoordTupleSize; ++j )
            vert.texCoords[ i ][ j ] = a.texCoords[ i ][ j ] - b.texCoords[ i ][ j ];

    return vert;
}

VERTEX_TEMPLATE_DECL
INLINE VERTEX_TEMPLATE_TYPE operator *( const VERTEX_TEMPLATE_TYPE& a, const VERTEX_TEMPLATE_TYPE& b )
{
    VERTEX_TEMPLATE_TYPE vert;

    for ( int32_t i = 0; i < positionTupleSize; ++i )
        vert.position[ i ] = a.position[ i ] * b.position[ i ];

    a.BlendColor( vert.color, b );

    for ( int32_t i = 0; i < normalTupleSize; ++i )
        vert.normal[ i ] = a.normal[ i ] * b.normal[ i ];

    for ( int32_t i = 0; i < numTexCoords; ++i )
        for ( int32_t j = 0; j < texCoordTupleSize; ++j )
            vert.texCoords[ i ][ j ] = a.texCoords[ i ][ j ] * b.texCoords[ i ][ j ];

    return vert;
}

VERTEX_TEMPLATE_DECL
INLINE VERTEX_TEMPLATE_TYPE& operator +=( const VERTEX_TEMPLATE_TYPE& a, const VERTEX_TEMPLATE_TYPE& b )
{
    for ( int32_t i = 0; i < positionTupleSize; ++i )
        a.position[ i ] += b.position[ i ];

    a.BlendColor( a.color, b );

    for ( int32_t i = 0; i < normalTupleSize; ++i )
        a.normal[ i ] += b.normal[ i ];

    for ( int32_t i = 0; i < numTexCoords; ++i )
        for ( int32_t j = 0; j < texCoordTupleSize; ++j )
            a.texCoords[ i ][ j ] += b.texCoords[ i ][ j ];

    return a;
}

VERTEX_TEMPLATE_DECL
INLINE bool operator ==( const VERTEX_TEMPLATE_TYPE& a, const VERTEX_TEMPLATE_TYPE& b )
{
    for ( int32_t i = 0; i < positionTupleSize; ++i )
        if ( a.position[ i ] != b.position[ i ] )
            return false;

    for ( int32_t i = 0; i < colorTupleSize; ++i )
        if ( a.color[ i ] != b.color[ i ] )
            return false;

    for ( int32_t i = 0; i < normalTupleSize; ++i )
        if ( a.normal[ i ] != b.normal[ i ] )
            return false;

    for ( int32_t i = 0; i < numTexCoords; ++i )
        for ( int32_t j = 0; j < texCoordTupleSize; ++j )
            if ( a.texCoords[ i ][ j ] != b.texCoords[ i ][ j ] )
                return false;

    return a;
}

//-------------------------------------------------------------------------------------------------------------
// bspVertex_t: specialized for the quake 3 renderer
//-------------------------------------------------------------------------------------------------------------

struct bspVertex_t
{
    glm::vec3 position;
    glm::vec2 texCoords[ 2 ];
    glm::vec3 normal;
    glm::u8vec4 color;
};

static INLINE glm::u8vec4 BlendColor( const glm::u8vec4& a, const glm::u8vec4& b )
{
    /*
    float aaNorm = a[ 3 ] * INVERSE_255;
    float alpha = aaNorm + b[ 3 ] * INVERSE_255 * ( 1.0f - aaNorm );
    float inverseAlpha = 1.0f / alpha;

    glm::vec3 anorm( glm::vec3( a ) * INVERSE_255 );
    glm::vec3 bnorm( glm::vec3( b ) * INVERSE_255 );

    glm::vec3 rgb( ( anorm * aaNorm + bnorm * ( 1.0f - aaNorm ) ) * inverseAlpha );
    */

    glm::vec4 x( glm::vec4( a ) * INVERSE_255 );
    glm::vec4 y( glm::vec4( b ) * INVERSE_255 );
    glm::vec4 rgb( x * y );

    glm::u8vec4 ret;

    ret.r = ( uint8_t )( rgb.r * 255.0f );
    ret.g = ( uint8_t )( rgb.g * 255.0f );
    ret.b = ( uint8_t )( rgb.b * 255.0f );
    ret.a = ( uint8_t )( rgb.a * 255.0f );

    return ret;
}

static INLINE bspVertex_t operator +( const bspVertex_t& a, const bspVertex_t& b )
{
    bspVertex_t vert;

    vert.position = a.position + b.position;
    vert.color = BlendColor( a.color, b.color );
    vert.normal = a.normal + b.normal;
    vert.texCoords[ 0 ] = a.texCoords[ 0 ] + b.texCoords[ 0 ];
    vert.texCoords[ 1 ] = a.texCoords[ 1 ] + b.texCoords[ 1 ];

    return vert;
}

static INLINE bspVertex_t operator -( const bspVertex_t& a, const bspVertex_t& b )
{
    bspVertex_t vert;

    vert.position = a.position - b.position;
    vert.color = BlendColor( b.color, a.color );
    vert.normal = a.normal - b.normal;
    vert.texCoords[ 0 ] = a.texCoords[ 0 ] - b.texCoords[ 0 ];
    vert.texCoords[ 1 ] = a.texCoords[ 1 ] - b.texCoords[ 1 ];

    return vert;
}

static INLINE bspVertex_t operator *( const bspVertex_t& a, float b )
{
    bspVertex_t vert;

    vert.position = a.position * b;
    vert.normal = a.normal * b;
    vert.texCoords[ 0 ] = a.texCoords[ 0 ] * b;
    vert.texCoords[ 1 ] = a.texCoords[ 1 ] * b;

    vert.color = a.color;

    return vert;
}

static INLINE bspVertex_t& operator += ( bspVertex_t& a, const bspVertex_t& b )
{
    a.position += b.position;
    a.normal += b.normal;
    a.texCoords[ 0 ] += b.texCoords[ 0 ];
    a.texCoords[ 1 ] += b.texCoords[ 1 ];

    return a;
}

static INLINE bool operator == ( const bspVertex_t&a, const bspVertex_t& b )
{
    return a.position == b.position
        && a.texCoords[ 0 ] == b.texCoords[ 0 ]
        && a.texCoords[ 1 ] == b.texCoords[ 1 ]
        && a.normal == b.normal
        && a.color == b.color;
}


//-------------------------------------------------------------------------------



