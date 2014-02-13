#include "math_util.h"

glm::vec3 GetBoundsCenter( vec3i& max, vec3i& min )
{
    glm::vec3 vmax( ( float ) max.x, ( float ) max.y, ( float ) max.z );
    glm::vec3 vmin( ( float ) min.x, ( float ) min.y, ( float ) min.z );

    return vmin + ( vmax - vmin ) / 2.0f;
}

glm::fquat MakeQuat( float angRad, const glm::vec3& axis )
{
    float halfTheta = angRad * 0.5f;
    float sinTheta = glm::sin( halfTheta );

    float w = glm::cos( halfTheta );

    float x = sinTheta * axis.x;
    float y = sinTheta * axis.y;
    float z = sinTheta * axis.z;

    return glm::fquat( w, x, y, z );
}
