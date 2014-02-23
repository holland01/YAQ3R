#pragma once

#include "common.h"
#include "plane.h"

#define FRUST_NUM_PLANES 6

enum
{
    FRUST_NONE      = 6,
    FRUST_TOP       = 0,
    FRUST_BOTTOM    = 1,
    FRUST_RIGHT     = 2,
    FRUST_LEFT      = 3,
    FRUST_NEAR      = 4,
    FRUST_FAR       = 5
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
