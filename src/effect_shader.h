#pragma once

#include "common.h"
#include "def.h"

struct mapData_t;

#define SHADER_MAX_NUM_STAGES 8 
#define SHADER_MAX_TOKEN_CHAR_LENGTH 64

// Info can be obtained from http://toolz.nexuizninjaz.com/shader/

enum surfaceParms_t
{
	SURFPARM_ALPHA_SHADOW		= 1 << 1,
	SURFPARM_AREA_PORTAL		= 1 << 2,
	SURFPARM_CLUSTER_PORTAL		= 1 << 3,
	SURFPARM_DO_NOT_ENTER		= 1 << 4,
	SURFPARM_FLESH				= 1 << 5,
	SURFPARM_FOG				= 1 << 6,
	SURFPARM_LAVA				= 1 << 7,
	SURFPARM_METAL_STEPS		= 1 << 8,
	SURFPARM_NO_DMG				= 1 << 9,
	SURFPARM_NO_DLIGHT			= 1 << 10,
	SURFPARM_NO_DRAW			= 1 << 11,
	SURFPARM_NO_DROP			= 1 << 12,
	SURFPARM_NO_IMPACT			= 1 << 13,
	SURFPARM_NO_MARKS			= 1 << 14,
	SURFPARM_NO_LIGHTMAP		= 1 << 15,
	SURFPARM_NO_STEPS			= 1 << 16,
	SURFPARM_NON_SOLID			= 1 << 17,
	SURFPARM_ORIGIN				= 1 << 18,
	SURFPARM_PLAYER_CLIP		= 1 << 19,
	SURFPARM_SLICK				= 1 << 20,
	SURFPARM_SLIME				= 1 << 21,
	SURFPARM_STRUCTURAL			= 1 << 22,
	SURFPARM_TRANS				= 1 << 23,
	SURFPARM_WATER				= 1 << 24
};	

enum rgbGen_t
{
	RGBGEN_UNDEFINED = 0,
	RGBGEN_VERTEX = 1,
	RGBGEN_ONE_MINUS_VERTEX,
	RGBGEN_IDENTITY_LIGHTING,
	RGBGEN_IDENTITY,
	RGBGEN_ENTITY,
	RGBGEN_ONE_MINUS_ENTITY,
	RGBGEN_DIFFUSE_LIGHTING,
	RGBGEN_WAVE
};

enum mapCmd_t
{
	MAP_CMD_UNDEFINED = 0,
	MAP_CMD_CLAMPMAP,
	MAP_CMD_MAP
};

enum mapType_t
{
	MAP_TYPE_IMAGE = 0,
	MAP_TYPE_LIGHT_MAP,
	MAP_TYPE_WHITE_IMAGE
};

using glHandleMap_t = std::map< std::string, GLint >;
using glHandleMapEntry_t = std::pair< std::string, GLint >;

struct shaderStage_t
{
	uint8_t	isStub; // if true, stage functionality is unsupported; fallback to default rendering process

	GLuint programID;
	GLuint textureObj;
	GLuint samplerObj;
	GLuint texOffset;

	GLenum blendSrc;
	GLenum blendDest;

	rgbGen_t rgbGen;
	mapCmd_t mapCmd;
	mapType_t mapType;

	float alphaGen; // if 0, use 1.0

	char mapArg[ SHADER_MAX_TOKEN_CHAR_LENGTH ];
	
	glHandleMap_t uniforms;
};

struct shaderInfo_t
{
	char name[ SHADER_MAX_TOKEN_CHAR_LENGTH ];

	uint8_t hasLightmap;
	uint8_t hasPolygonOffset;

	GLuint samplerObj;
	GLuint textureObj;

	uint32_t surfaceParms;
	int stageCount;
	int stageStubCount;

	float surfaceLight; // 0 if no light

	shaderStage_t stageBuffer[ SHADER_MAX_NUM_STAGES ];
};

using shaderMap_t = std::map< std::string, shaderInfo_t >;
using shaderMapEntry_t = std::pair< std::string, shaderInfo_t >;

void LoadShaders( const mapData_t* map, uint32_t loadFlags, shaderMap_t& effectShaders );