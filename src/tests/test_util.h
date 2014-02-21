#pragma once

#include "../common.h"

class InputCamera;

void OnKeyPress( GLFWwindow* window, int key, int scancode, int action, int mods, InputCamera* const camera, bool& cursorVisible );
