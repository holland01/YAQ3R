#pragma once

/*
==========================

Author: Holland Schutte
License: WTFPL

    common.h

Global include file, containing often-used or down-right-necessary files for each module.

==========================
*/

#include <Windows.h> // This needs to be before GLFW includes to prevent APIENTRY macro redef error
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

#include <string>
#include <sstream>
#include <vector>
#include <functional>
#include <map>
#include <utility>
#include <stack>

#define GLM_FORCE_RADIANS

#include <glm/glm.hpp>
#include <glm/common.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "vec.h"

INLINE bool FileGetExt( std::string& outExt, const std::string& filename  )
{
	// Second condition is to ensure we actually have a file extension we can use
	size_t index;
	if ( ( index = filename.find_last_of( '.' ) ) != std::string::npos && index != filename.size() - 1 )
	{
		outExt = filename.substr( index + 1 );
		return true;
	}
	return index >= 0;
}
