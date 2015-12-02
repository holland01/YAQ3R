#pragma once

#include "common.h"
#include "def.h"
#include "glutil.h"
#include <memory>

struct mapData_t;

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
    RGBGEN_UNDEFINED = 0xFF,
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
        : name( KEY_UNDEFINED ),
          data()
	{
	}

    effect_t( const std::string& name_ )
        : name( name_ ),
          data()
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

#define SHADER_MAX_TOKEN_CHAR_LENGTH 64

struct shaderStage_t
{
    bool						depthPass = false;
	
    int32_t						textureIndex = INDEX_UNDEFINED;

    texCoordGen_t				tcgen = TCGEN_BASE; // Normal texcoord generation (if textureIndex is defined)

    GLenum						blendSrc = GL_ONE; // src is incoming pixels

    GLenum						blendDest = GL_ZERO; // destination is the pixels which are already within the framebuffer from the previous frame

    GLenum						depthFunc = GL_LEQUAL; // Default is LEQUAL

    rgbGen_t					rgbGen = RGBGEN_UNDEFINED; // how colors are generated - identity refers to a white image

    alphaFunc_t					alphaFunc = ALPHA_FUNC_UNDEFINED;  // alpha constraint for the fragment; if fragment fails the condition, we discard it

    mapCmd_t					mapCmd = MAP_CMD_UNDEFINED; // either "map" or "clampmap"; clampmap refers to a non-repeating texture

    mapType_t					mapType = MAP_TYPE_UNDEFINED; // can be an image, a lightmap, or a whiteimage

    std::vector< effect_t >		effects;  // dynamic effects, designed to be passed to the shader

    float						alphaGen = 0.0f; // if 0, assume an alpha value of 1

    std::array< char, SHADER_MAX_TOKEN_CHAR_LENGTH > texturePath = {{ 0 }}; // path to the texture image, if we have one

    std::shared_ptr< Program >	program; // handle to our generated program
};

struct shaderInfo_t
{
    bool				deform = false; // implies an animated vertex deformation
	
    vertexDeformCmd_t	deformCmd = VERTEXDEFORM_CMD_UNDEFINED; // choose wave, normal, or bulge

    vertexDeformFunc_t	deformFn = VERTEXDEFORM_FUNC_UNDEFINED; // arbitrary sinusoidal functions which are applied by the command (e.g., sawtooth, triangle, a normal sine wave, etc)

    effect_t            deformParms; // arbitrary parameters for our deform

    GLenum				cullFace = GL_FALSE;

    uint32_t			surfaceParms = 0; // global surface parameters

    uint32_t			localLoadFlags = 0; // we pass a list of global flags we'd like to see applied everywhere, however some shaders may contradict this

    float				tessSize = 0.0f; // 0 if none

    int					stageCount = 0; // the amount of draw passes for this shader entry

    float				surfaceLight = 0.0f; // 0 if no light

    std::array< char, SHADER_MAX_TOKEN_CHAR_LENGTH > name = {{ 0 }};
	
    std::vector< shaderStage_t > stageBuffer;
};

using shaderMap_t = std::map< std::string, shaderInfo_t >;
using shaderMapEntry_t = std::pair< std::string, shaderInfo_t >;

glm::ivec2 S_LoadShaders( const mapData_t* map, std::vector< texture_t >& textures, shaderMap_t& effectShaders, uint32_t loadFlags );

static INLINE bool S_StageHasIdentityColor( const shaderStage_t& s )
{
	return s.rgbGen == RGBGEN_IDENTITY || s.rgbGen == RGBGEN_IDENTITY_LIGHTING;
}
