#pragma once

#include "common.h"

class EuAng
{
public:

    float pitch, yaw, roll;

    EuAng( void );

    EuAng( float p, float y, float r );

    glm::vec3   ToVector( bool inRadians = false ) const;

    void        Normalize( void );
};


INLINE bool operator== ( const EuAng& a, const EuAng& b )
{
    return a.pitch == b.pitch && a.yaw == b.yaw && a.roll == b.roll;
}

INLINE bool operator!= ( const EuAng& a, const EuAng& b )
{
    return !( a == b );
}
