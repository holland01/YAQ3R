#pragma once

#include "common.h"

class Q3BspMap;

struct Q3MModel
{
    int         face;
    glm::mat4   localTrans;
    glm::vec3   boundsCenter;
};

Q3MModel* GenModelTransform( int face, Q3BspMap* map );

