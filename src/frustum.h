#pragma once

#include "common.h"
#include "plane.h"

#define FRUST_NUM_PLANES 6

enum
{
    FRUST_NONE      = 0x0,
    FRUST_TOP       = 0x1,
    FRUST_BOTTOM    = 0x2,
    FRUST_RIGHT     = 0x4,
    FRUST_LEFT      = 0x8,
    FRUST_NEAR      = 0x10,
    FRUST_FAR       = 0x20
};

struct  viewParams_t;
class   AABB;

class Frustum
{
    plane_t     frustPlanes[ FRUST_NUM_PLANES ];

public:

    Frustum( void );

    ~Frustum( void );

    void    Update( const viewParams_t& params );

    bool    IntersectsBox( const AABB& box );
};
