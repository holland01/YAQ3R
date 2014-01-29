#pragma once

#include "../common.h"
#include "../renderer.h"

void loadTestRenderer( GLFWwindow* window );

void drawTestRenderer( void );

void handleInputTestRenderer( GLFWwindow* w, int key, int scancode, int action, int mods );

void handleMousePosTestRenderer( GLFWwindow* w, double x, double y );
