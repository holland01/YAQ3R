#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#include <string>
#include <set>
#include <bitset>

#define GLM_FORCE_RADIANS

#include <glm/glm.hpp>
#include <glm/common.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "vec.h"

#define NOP ;

typedef unsigned char byte;

void flagExit( void );
