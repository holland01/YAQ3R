#pragma once

#include "common.h"
#include "def.h"

struct mapData_t;

struct renderTexture_t
{
	int index;
};

void LoadShaders( mapData_t* map );