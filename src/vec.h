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


