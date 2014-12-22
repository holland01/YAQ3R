#pragma once


/*
=====================================================

Author: Holland Schutte
License: WTFPL
                   vec.h

        Generic vector structures,
        for use in C-style structs
        being read into from binary
        data.

        Created primarily to enhance
        readability.

=====================================================
*/

struct color4f_t
{
    float r, g, b, a;
};

struct vec3f_t
{
    float x, y, z;
};

struct vec3i_t
{
    int x, y, z;
};

struct vec2f_t
{
    float x, y;
};

INLINE vec3f_t operator * ( const vec3f_t& a, float b )
{
	vec3f_t r = 
	{
		a.x * b,
		a.y * b,
		a.z * b
	};

	return r;
}

INLINE vec3f_t operator + ( const vec3f_t& a, const vec3f_t& b )
{
	vec3f_t r = 
	{
		a.x + b.x,
		a.y + b.y,
		a.z + b.z
	};

	return r;
}

INLINE vec2f_t operator * ( const vec2f_t& a, float b )
{
	vec2f_t r = 
	{
		a.x * b,
		a.y * b
	};

	return r;
}

INLINE vec2f_t operator + ( const vec2f_t& a, const vec2f_t& b )
{
	vec2f_t r = 
	{
		a.x + b.x,
		a.y + b.y,
	};

	return r;
}




