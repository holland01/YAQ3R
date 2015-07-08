#pragma once

#include "common.h"
#include "def.h"
#include "glutil.h"
#include <memory>

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
	SURFPARM_WATER				= 1 << 24,
	SURFPARM_ENVMAP				= 1 << 25 // additional; not part of the standarad AFAIK
};	

enum vertexDeformCmd_t
{
	VERTEXDEFORM_CMD_UNDEFINED = 0xFF,
	VERTEXDEFORM_CMD_WAVE = 0,
	VERTEXDEFORM_CMD_NORMAL,
	VERTEXDEFORM_CMD_BULGE
};

enum vertexDeformFunc_t
{
	VERTEXDEFORM_FUNC_UNDEFINED = 0xFF,
	VERTEXDEFORM_FUNC_TRIANGLE,
	VERTEXDEFORM_FUNC_SIN,
	VERTEXDEFORM_FUNC_SQUARE,
	VERTEXDEFORM_FUNC_SAWTOOTH,
	VERTEXDEFORM_FUNC_INV_SAWTOOTH,
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

enum texCoordGen_t
{
	TCGEN_BASE = 0,
	TCGEN_LIGHTMAP,
	TCGEN_ENVIRONMENT,
	TCGEN_VECTOR
};

struct normal_t
{
	float x;
	float y;
};

struct wave_t 
{
	float spread;
	float base;
	float amplitude;
	float phase;
	float frequency;
};

struct bulge_t
{
	float width;
	float height;
	float speed;
};

struct effect_t
{
	std::string name;

	union data_t
	{
		struct 
		{
			float transform[ 2 ][ 2 ];
			float center[ 2 ];
		}
		rotation2D;

		float scale2D[ 2 ][ 2 ];

		wave_t	wave;
		bulge_t bulge;
		normal_t normal;
		float	xyzw[ 4 ];
	} data;
	 
	effect_t( void )
		: data()
	{
	}

	effect_t( const effect_t& e )
		: name( e.name ),
		  data( e.data )
	{
	}

	~effect_t( void )
	{}
};

struct shaderStage_t
{
	bool						depthPass;
	
	int32_t						textureIndex;

	texCoordGen_t				tcgen;

	GLenum						rgbSrc;
	GLenum						rgbDest;

	GLenum						alphaSrc;
	GLenum						alphaDest;

	GLenum						depthFunc; // Default is LEQUAL

	rgbGen_t					rgbGen;
	alphaFunc_t					alphaFunc;
	mapCmd_t					mapCmd;
	mapType_t					mapType;

	std::vector< effect_t >		effects;

	float						alphaGen; // if 0, use 1.0

	char						texturePath[ SHADER_MAX_TOKEN_CHAR_LENGTH ];

	std::shared_ptr< Program >	program;

	shaderStage_t( void );
};

struct shaderInfo_t
{
	bool				deform: 1;
	
	vertexDeformCmd_t	deformCmd;
	vertexDeformFunc_t	deformFn;
	wave_t				deformParms;	

	GLenum				cullFace;
	uint32_t			surfaceParms;
	uint32_t			loadFlags; // we pass a list of global flags we'd like to see applied everywhere, however some shaders may contradict this

	float				tessSize; // 0 if none
	int					stageCount;

	float				surfaceLight; // 0 if no light

	char				name[ SHADER_MAX_TOKEN_CHAR_LENGTH ];
	
	std::array< shaderStage_t, SHADER_MAX_NUM_STAGES > stageBuffer;

	shaderInfo_t( void );
};

using shaderMap_t = std::map< std::string, shaderInfo_t >;
using shaderMapEntry_t = std::pair< std::string, shaderInfo_t >;

glm::ivec2 Shader_LoadAll( const mapData_t* map, std::vector< texture_t >& textures, shaderMap_t& effectShaders, uint32_t loadFlags );

static INLINE bool Shader_StageHasIdentityColor( const shaderStage_t& s )
{
	return s.rgbGen == RGBGEN_IDENTITY || s.rgbGen == RGBGEN_IDENTITY_LIGHTING;
}