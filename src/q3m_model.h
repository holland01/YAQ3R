#pragma once

#include "common.h"

class Quake3Map;

struct Q3MModel
{
    int         face;
    glm::mat4   localTrans;
    glm::vec3   boundsCenter;
};

Q3MModel* GenModelTransform( int face, Quake3Map* map );

