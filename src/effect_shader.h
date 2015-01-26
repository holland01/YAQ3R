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

enum vertexDeformCmd_t
{
	VERTEXDEFORM_CMD_UNDEFINED = 0,
	VERTEXDEFORM_CMD_WAVE,
	VERTEXDEFORM_CMD_NORMAL,
	VERTEXDEFORM_CMD_BULGE
};

enum vertexDeformFunc_t
{
	VERTEXDEFORM_FUNC_UNDEFINED = 0,
	VERTEXDEFORM_FUNC_TRIANGLE,
	VERTEXDEFORM_FUNC_SIN,
	VERTEXDEFORM_FUNC_SQUARE,
	VERTEXDEFORM_FUNC_SAWTOOTH,
	VERTEXDEFORM_FUNC_INV_SAWTOOTH
};

enum rgbGen_t
{
	//RGBGEN_UNDEFINED = 0,
	RGBGEN_VERTEX = 0,
	RGBGEN_ONE_MINUS_VERTEX,
	RGBGEN_IDENTITY_LIGHTING,
	RGBGEN_IDENTITY,
	RGBGEN_ENTITY,
	RGBGEN_ONE_MINUS_ENTITY,
	RGBGEN_DIFFUSE_LIGHTING,
	RGBGEN_WAVE
};

enum alphaFunc_t
{
	ALPHA_FUNC_UNDEFINED = 0,
	ALPHA_FUNC_GEQUAL_128, // will pass fragment test if alpha value is >= ( 128 / 255 )
	ALPHA_FUNC_GTHAN_0, // will pass fragment test if alpha value is > 0
	ALPHA_FUNC_LTHAN_128 // will pass fragment test if alpha value is < ( 128 / 255 )
};

enum mapCmd_t
{
	MAP_CMD_UNDEFINED = 0,
	MAP_CMD_CLAMPMAP,
	MAP_CMD_MAP
};

enum mapType_t
{
	MAP_TYPE_UNDEFINED = 0,
	MAP_TYPE_IMAGE,
	MAP_TYPE_LIGHT_MAP,
	MAP_TYPE_WHITE_IMAGE
};

struct shaderStage_t
{
	uint8_t	isStub; // if true, stage functionality is unsupported; fallback to default rendering process
	uint8_t isDepthPass;
	uint8_t hasTexMod;

	GLuint programID;
	GLuint textureObj;
	GLuint samplerObj;
	GLuint texOffset;

	GLenum blendSrc;
	GLenum blendDest;
	GLenum depthFunc; // Default is LEQUAL

	rgbGen_t rgbGen;
	alphaFunc_t alphaFunc;
	mapCmd_t mapCmd;
	mapType_t mapType;

	float alphaGen; // if 0, use 1.0

	char mapArg[ SHADER_MAX_TOKEN_CHAR_LENGTH ];
	
	glHandleMap_t uniforms;

	std::vector< glm::mat2 > texTransformList;
	glm::mat2 texTransform;

	shaderStage_t( void );
};

struct shaderInfo_t
{
	uint8_t hasLightmap;
	uint8_t hasPolygonOffset;
	
	vertexDeformCmd_t	deformCmd;

	union {
		// Any cmd which is not bulge
		struct {
			vertexDeformFunc_t	deformFn;
			float				deformDiv; // the wave spread for the vertex deformation
			float				deformBase; // game unit distance in which the vertex is deplaced from the surface
			float				deformAmplitude;
			float				deformPhase;
			float				deformFrequency;
		};

		// bulge only
		struct {
			float				deformBulgeWidth;
			float				deformBulgeHeight;
			float				deformBulgeSpeed;
		};
	};
	GLuint samplerObj;
	GLuint textureObj;

	uint32_t surfaceParms;
	float tessSize; // 0 if none
	int stageCount;

	float surfaceLight; // 0 if no light

	char name[ SHADER_MAX_TOKEN_CHAR_LENGTH ];
	shaderStage_t stageBuffer[ SHADER_MAX_NUM_STAGES ];

	shaderInfo_t( void );
};

using shaderMap_t = std::map< std::string, shaderInfo_t >;
using shaderMapEntry_t = std::pair< std::string, shaderInfo_t >;

void LoadShaders( const mapData_t* map, uint32_t loadFlags, shaderMap_t& effectShaders );