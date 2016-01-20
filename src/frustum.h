#pragma once

#include "common.h"
#include "plane.h"
#include "io.h"

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

	mutable uint32_t acceptCount;

	mutable uint32_t rejectCount;

	glm::mat4 mvp;

	glm::vec4 CalcPlaneFromOrigin( const glm::vec4& position, const glm::vec4& origin );

public:

    Frustum( void );

    ~Frustum( void );

    void    Update( const viewParams_t& params, bool normalizeDistance );

	void	PrintMetrics( void ) const;

	void	ResetMetrics( void ) const { rejectCount = 0; acceptCount = 0; }

    bool    IntersectsBox( const AABB& box ) const;
};

INLINE void Frustum::PrintMetrics( void ) const
{
	printf( "Reject Count: %iu; Accept Count: %iu\r", rejectCount, acceptCount );
}
