#pragma once

#include "common.h"
#include "render_data.h"
#include "renderer/renderer_local.h"
#include "renderer/program.h"
#include <unordered_map>
#include <memory>

//struct bspVertex_t;

// BSP file meta-data
enum
{
	BSP_HEX_ID = 0x49425350,
	BSP_NUM_ENTRIES = 17,
	BSP_Q3_VERSION = 0x2E,

	BSP_LUMP_ENTITIES = 0x00,
	BSP_LUMP_SHADERS = 0x01,
	BSP_LUMP_PLANES = 0x02,
	BSP_LUMP_NODES = 0x03,
	BSP_LUMP_LEAVES = 0x04,
	BSP_LUMP_LEAF_FACES = 0x05,
	BSP_LUMP_LEAF_BRUSHES = 0x06,
	BSP_LUMP_MODELS = 0x07,
	BSP_LUMP_BRUSHES = 0x08,
	BSP_LUMP_BRUSH_SIDES = 0x09,
	BSP_LUMP_VERTEXES = 0x0A,
	BSP_LUMP_MESH_VERTEXES = 0x0B,
	BSP_LUMP_FOGS = 0x0C,
	BSP_LUMP_FACES = 0x0D,
	BSP_LUMP_LIGHTMAPS = 0x0E,
	BSP_LUMP_LIGHTVOLS = 0x0F,
	BSP_LUMP_VISDATA = 0x10,

	BSP_FACE_TYPE_POLYGON = 0x1,
	BSP_FACE_TYPE_PATCH = 0x2,
	BSP_FACE_TYPE_MESH = 0x3,
	BSP_FACE_TYPE_BILLBOARD = 0x4,

	BSP_LIGHTMAP_WIDTH = 128,
	BSP_LIGHTMAP_HEIGHT = 128,

	BSP_NUM_CONTROL_POINTS = 9,

	// Stolen from ioquake's code/qcommon/surfaceflags.h

	BSP_CONTENTS_SOLID = 0x1,
	BSP_CONTENTS_LAVA = 0x8,
	BSP_CONTENTS_SLIME = 0x10,
	BSP_CONTENTS_WATER = 0x20,
	BSP_CONTENTS_FOG = 0x40,

	BSP_CONTENTS_NOTTEAM1 = 0x0080,
	BSP_CONTENTS_NOTTEAM2 = 0x0100,
	BSP_CONTENTS_NOBOTCLIP = 0x0200,

	BSP_CONTENTS_AREAPORTAL = 0x8000,
	BSP_CONTENTS_PLAYERCLIP = 0x10000,
	BSP_CONTENTS_MONSTERCLIP = 0x20000,

	BSP_CONTENTS_TELEPORTER = 0x40000,
	BSP_CONTENTS_JUMPPAD = 0x80000,
	BSP_CONTENTS_CLUSTERPORTAL = 0x100000,
	BSP_CONTENTS_DONOTENTER = 0x200000,
	BSP_CONTENTS_BOTCLIP = 0x400000,
	BSP_CONTENTS_MOVER = 0x800000,

	BSP_CONTENTS_ORIGIN = 0x1000000, // removed before bsping an entity

	BSP_CONTENTS_BODY = 0x2000000,
	BSP_CONTENTS_CORPSE = 0x4000000,
	BSP_CONTENTS_DETAIL = 0x8000000, // brush: not used for the bsp
	BSP_CONTENTS_STRUCTURAL = 0x10000000, // brush: used for the bsp
	BSP_CONTENTS_TRANSLUCENT = 0x20000000, // don't consume surface fragments inside
	BSP_CONTENTS_TRIGGER = 0x40000000, //
	BSP_CONTENTS_NODROP = 0x80000000,

	BSP_SURFACE_NODAMAGE = 0x1, // never give falling damage
	BSP_SURFACE_SLICK = 0x2, // effects game physics
	BSP_SURFACE_SKY = 0x4, // lighting from environment map
	BSP_SURFACE_LADDER = 0x8,
	BSP_SURFACE_NOIMPACT = 0x10, // don't make missile explosions
	BSP_SURFACE_NOMARKS = 0x20, // don't leave missile marks
	BSP_SURFACE_FLESH = 0x40, // make flesh sounds and effects
	BSP_SURFACE_NODRAW = 0x80, // don't generate a drawsurface at all
	BSP_SURFACE_HINT = 0x100, // make a primary bsp splitter
	BSP_SURFACE_SKIP = 0x200, // completely ignore, allowing non-closed brushes
	BSP_SURFACE_NOLIGHTMAP = 0x400, // surface doesn't need a lightmap
	BSP_SURFACE_POINTLIGHT = 0x800, // generate light info at vertexes
	BSP_SURFACE_METALSTEPS = 0x1000, // clanking footsteps
	BSP_SURFACE_NOSTEPS = 0x2000, // no footstep sounds
	BSP_SURFACE_NONSOLID = 0x4000, // don't collide against curves with this set
	BSP_SURFACE_LIGHTFILTER  = 0x8000, // act as a light filter during q3map
	BSP_SURFACE_ALPHASHADOW = 0x10000, // do per-pixel light shadow casting in q3map
	BSP_SURFACE_NODLIGHT = 0x20000, // don't dlight(?) even if solid (solid lava, skies)
	BSP_SURFACE_DUST = 0x40000 // leave a dust trail when walking
};

// Map loader-specific flags
enum
{
	Q3LOAD_TEXTURE_SRGB = 1,
	Q3LOAD_TEXTURE_ANISOTROPY = 1 << 2,
	Q3LOAD_TEXTURE_MIPMAP = 1 << 3,
	Q3LOAD_TEXTURE_ROTATE90CCW = 1 << 4,

	Q3LOAD_ALL = Q3LOAD_TEXTURE_SRGB | Q3LOAD_TEXTURE_ANISOTROPY | Q3LOAD_TEXTURE_MIPMAP
};

struct bspEntity_t
{
	char* infoString;
};

struct bspLump_t
{
	int offset;
	int length;
};

struct bspHeader_t
{
	char        id[ 4 ];
	int         version;
	bspLump_t   directories[ 17 ];
};

struct bspPlane_t
{
	glm::vec3       normal;
	float       distance;
};

struct bspNode_t
{
	int     plane;
	int     children[ 2 ]; // 0 => front, 1 => back

	glm::ivec3   boxMin;
	glm::ivec3   boxMax;
};

struct bspLeaf_t
{
	int clusterIndex;
	int areaPortal;

	glm::ivec3   boxMin;
	glm::ivec3   boxMax;

	int leafFaceOffset;
	int numLeafFaces;

	int leafBrushOffset;
	int numLeafBrushes;
};

struct bspLeafFace_t
{
	int index;
};

struct bspLeafBrush_t
{
	int index;
};

struct bspModel_t
{
	glm::vec3 boxMin;
	glm::vec3 boxMax;

	int faceOffset;
	int numFaces;

	int brushOffset;
	int numBrushes;
};

struct bspBrush_t
{
	int brushSide;
	int numBrushSides;
	int shader; // refers to content flags
};

struct bspBrushSide_t
{
	int plane;	// positive plane side faces out of the brush surface
	int shader;
};

struct bspShader_t
{
	char    name[ 64 ];
	int     surfaceFlags;
	int     contentsFlags;
};

struct bspMeshVertex_t
{
	int offset;
};

struct bspFog_t
{
	char    name[ 64 ];
	int     brush;
	int     visibleSide;
};

struct bspFace_t
{
	int shader;
	int fog;
	int type;

	int vertexOffset;
	int numVertexes;

	int meshVertexOffset;
	int numMeshVertexes;

	int lightmapIndex;
	int lightmapStartCorner[ 2 ];
	int lightmapSize[ 2 ];

	glm::vec3 lightmapOrigin; // in world space
	glm::vec3 lightmapStVecs[ 2 ]; // for patches, 0 and 1 are lod vectors
	glm::vec3 normal;

	int patchDimensions[ 2 ];
};

struct bspLightmap_t
{
	unsigned char map[ BSP_LIGHTMAP_WIDTH ][ BSP_LIGHTMAP_HEIGHT ][ 3 ]; // lightmap color data. RGB.
};

struct bspLightvol_t
{
	glm::u8vec3 ambient;		// RGB color
	glm::u8vec3 directional;	// RGB color
	glm::u8vec2 direction;	// - to light; 0 => phi, 1 => theta
};

struct bspVisdata_t
{
	int     numVectors;
	int     sizeVector;

	unsigned char*   bitsets;
};

struct mapData_t
{
	bspHeader_t	header;

	std::vector< char > entitiesSrc;
	std::vector< bspShader_t > shaders;
	std::vector< bspPlane_t > planes;
	std::vector< bspNode_t > nodes;
	std::vector< bspLeaf_t > leaves;
	std::vector< bspLeafFace_t > leafFaces;
	std::vector< bspLeafBrush_t > leafBrushes;
	std::vector< bspModel_t > models;
	std::vector< bspBrush_t > brushes;
	std::vector< bspBrushSide_t	> brushSides;
	std::vector< bspVertex_t > vertexes;
	std::vector< bspMeshVertex_t > meshVertexes;
	std::vector< bspFog_t > fogs;
	std::vector< bspFace_t > faces;
	std::vector< bspLightmap_t > lightmaps;
	std::vector< bspLightvol_t > lightvols;
	std::vector< unsigned char > bitsetSrc;

	bspEntity_t entities;
	bspVisdata_t visdata;

	int                 entityStringLen;

	int                 numNodes;

	int                 numLeaves;
	int                 numLeafFaces;
	int					numLeafBrushes;

	int                 numPlanes;

	int                 numVertexes;

	int					numBrushes;
	int					numBrushSides;

	int                 numShaders;
	int                 numModels;

	int                 numFogs;
	int                 numFaces;

	int                 numMeshVertexes;

	int					numLightmaps;
	int					numLightvols;

	int                 numVisdataVecs;

	std::string			basePath; // root directory of the map
};

// --------------------------------------------------------
// GL-specific
// --------------------------------------------------------

struct triangle_t
{
	GLuint indices[ 3 ];
};

struct leafModel_t
{
	std::vector< int > modelIndices;
};

// --------------------------------------------------------
// Effect Shaders
// --------------------------------------------------------

struct mapData_t;


// Info can be obtained from http://toolz.nexuizninjaz.com/shader/
//
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

enum effectType_t
{
	EFFECT_UNDEFINED = 0xFF,
	EFFECT_WAVE = 0,
	EFFECT_BULGE,
	EFFECT_NORMAL,
	EFFECT_ROTATION2D,
	EFFECT_SCALE2D,
	EFFECT_XYZW
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
	effectType_t type;

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
		  type( EFFECT_UNDEFINED ),
		  data()
	{
	}

	effect_t( const std::string& name_ )
		: name( name_ ),
		  type( EFFECT_UNDEFINED ),
		  data()
	{
	}

	effect_t( const effect_t& e )
		: name( e.name ),
		  type( e.type ),
		  data( e.data )
	{
	}
};

#define SHADER_MAX_TOKEN_CHAR_LENGTH 64

class Program;

struct shaderStage_t
{
	bool						depthPass = false;

	int32_t						textureIndex = INDEX_UNDEFINED;

	texCoordGen_t				tcgen = TCGEN_BASE; // Normal texcoord generation (if textureIndex is defined)

	GLenum						blendSrc = GL_ONE; // src is incoming pixels

	GLenum						blendDest = GL_ZERO; // destination is the pixels which are already within the framebuffer from the previous frame

	GLenum						depthFunc = GL_LEQUAL;

	rgbGen_t					rgbGen = RGBGEN_UNDEFINED; // how colors are generated - identity refers to a white image

	alphaFunc_t					alphaFunc = ALPHA_FUNC_UNDEFINED;  // alpha constraint for the fragment; if fragment fails the condition, we discard it

	mapCmd_t					mapCmd = MAP_CMD_UNDEFINED; // either "map" or "clampmap"; clampmap refers to a non-repeating texture

	mapType_t					mapType = MAP_TYPE_UNDEFINED; // can be an image, a lightmap, or a whiteimage

	std::vector< effect_t >		effects;  // dynamic effects, designed to be passed to the shader

	float						alphaGen = 0.0f; // if 0, assume an alpha value of 1

	std::array< char, SHADER_MAX_TOKEN_CHAR_LENGTH > texturePath; // path to the texture image, if we have one

	gProgramHandle_t program;

	shaderStage_t( void )
	//	: program( nullptr )
		: program( { ( uint16_t ) G_UNSPECIFIED } )
	{
		texturePath.fill( 0 );
	}

	const Program& GetProgram( void ) const { return *GQueryProgram( program ); }
};

struct shaderInfo_t
{
	bool				deform = false; // implies an animated vertex deformation

	vertexDeformCmd_t	deformCmd = VERTEXDEFORM_CMD_UNDEFINED; // choose wave, normal, or bulge

	vertexDeformFunc_t	deformFn = VERTEXDEFORM_FUNC_UNDEFINED; // arbitrary sinusoidal functions which are applied by the command (e.g., sawtooth, triangle, a normal sine wave, etc)

	effect_t            deformParms; // arbitrary parameters for our deform

	uint32_t			cullFace = G_UNSPECIFIED;

	uint32_t			surfaceParms = 0; // global surface parameters

	uint8_t				localLoadFlags;

	float				tessSize = 0.0f; // 0 if none

	int					stageCount = 0; // the amount of draw passes for this shader entry

	float				surfaceLight = 0.0f; // 0 if no light

	std::array< char, SHADER_MAX_TOKEN_CHAR_LENGTH > name;

	std::vector< shaderStage_t > stageBuffer;

	shaderInfo_t( void )
	{
		name.fill( 0 );
	}
};

bool EquivalentProgramTypes( const shaderStage_t* a, const shaderStage_t* b );

using shaderMap_t = std::unordered_map< std::string, shaderInfo_t >;
using shaderMapEntry_t = std::pair< std::string, shaderInfo_t >;
