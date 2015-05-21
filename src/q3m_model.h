#pragma once

#include "common.h"

struct bspVertex_t;

// BSP file meta-data
enum
{
    BSP_HEX_ID = 0x49425350,
    BSP_NUM_ENTRIES = 17,
    BSP_Q3_VERSION = 0x2E,

    BSP_LUMP_ENTITIES = 0x00,
    BSP_LUMP_TEXTURES = 0x01,
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
    BSP_LUMP_EFFECTS = 0x0C,
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

	BSP_NUM_CONTROL_POINTS = 9
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

struct triangle_t
{
	GLuint indices[ 3 ];
};

struct leafModel_t
{
	std::vector< int > modelIndices;
};

struct bezPatch_t
{
	GLuint						vbo;
	int							subdivLevel;

	const bspVertex_t*			controlPoints[ 9 ];

	std::vector< bspVertex_t >	vertices;
	std::vector< int  >			indices;
	std::vector< int* >			rowIndices;
	std::vector< int  >			trisPerRow;

	 bezPatch_t( void );
	~bezPatch_t( void );
};

struct mapModel_t
{
	GLuint				  vao;
	std::vector< GLuint > indices;
	std::vector< bezPatch_t* > patches;

	mapModel_t( void );
	~mapModel_t( void );
};

