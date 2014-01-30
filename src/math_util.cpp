#include "math_util.h"

glm::vec3 GetBoundsCenter( vec3i& max, vec3i& min )
{
    glm::vec3 vmax( ( float ) max.x, ( float ) max.y, ( float ) max.z );
    glm::vec3 vmin( ( float ) min.x, ( float ) min.y, ( float ) min.z );

    return vmin + ( vmax - vmin ) / 2.0f;
}
