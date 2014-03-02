#pragma once


/*
=====================================================

Author: Holland Schutte
License: WTFPL
                   vert.h

        Generic vertex structures,
        for use in C-style structs
        being read into from binary
        data.

        Created primarily to enhance
        readability.

=====================================================
*/

union float4_t
{
    struct {
        float r, g, b, a;
    };

    struct {
        float x, y, z, w;
    };
};

union float3_t
{
    struct {
        float r, g, b;
    };

    struct {
        float x, y, z;
    };
};

union float2_t
{
    struct {
        float s, t;
    };

    struct {
        float x, y;
    };
};

struct int3_t
{
    int x, y, z;
};




