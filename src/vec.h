#pragma once


/*
=====================================================

                   vec.h

        Generic vector structures,
        for use in C-style structs
        being read into from binary
        data.

        Created primarily to enhance
        readability.

=====================================================
*/

struct vec3f
{
    float x, y, z;
};

struct vec3i
{
    int x, y, z;
};

struct vec2f
{
    float x, y;
};

