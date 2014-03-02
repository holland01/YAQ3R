#pragma once

/*
==========================

Author: Holland Schutte
License: WTFPL

    common.h

Global include file, containing often-used or down-right-necessary files for each module.

==========================
*/

#include "def.h"
#include "global.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <stdarg.h>
#include <stddef.h>
#include <time.h>

#ifdef __linux__
#include <fts.h>
#include <err.h>
#include <ftw.h>
#include <unistd.h>
#include <dirent.h>
#endif

#include <string>
#include <sstream>
#include <vector>
#include <functional>
#include <map>
#include <utility>
#include <stack>
#include <algorithm>


#define GLM_FORCE_RADIANS

#include <glm/glm.hpp>
#include <glm/common.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "vert.h"

#define NOP ;
