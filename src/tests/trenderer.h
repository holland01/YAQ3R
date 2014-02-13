#pragma once

#include "../common.h"
#include "../renderer.h"

void BSPR_LoadTest( GLFWwindow* window );

void BSPR_DrawTest( void );

void BSPR_HandleKeyInput( GLFWwindow* w, int key, int scancode, int action, int mods );

void BSPR_HandleMouseMove( GLFWwindow* w, double x, double y );
