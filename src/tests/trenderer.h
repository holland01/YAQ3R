#pragma once

#include "../common.h"
#include "../renderer.h"

void LoadTestRenderer( GLFWwindow* window );

void DrawTestRenderer( void );

void HandleInputTestRenderer( GLFWwindow* w, int key, int scancode, int action, int mods );

void HandleMousePosTestRenderer( GLFWwindow* w, double x, double y );
