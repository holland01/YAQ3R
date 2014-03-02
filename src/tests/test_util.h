#pragma once

#include "../common.h"

class Camera;

void OnKeyPress( GLFWwindow* window, int key, int scancode, int action, int mods, Camera* const camera, bool& cursorVisible );
