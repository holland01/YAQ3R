#pragma once

#include "common.h"
#include "render_data.h"

//struct bspVertex_t;

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
    int     children[ 2 ];

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
	int texture;
};

struct bspBrushSide_t
{
	int plane;
	int texture;
};

/*
struct bspVertex_t
{
    glm::vec3 position;
    glm::vec2 texCoords[ 2 ]; // 0 => surface, 1 => lightmap
    glm::vec3 normal;

    glm::u8vec4 color;

	bspVertex_t( void );
	bspVertex_t( const glm::vec3& pos, const glm::vec3& norm, const glm::vec2& surfTexCoords, const glm::vec2& lightmapTexCoords, const glm::u8vec4& colors );
	bspVertex_t( const bspVertex_t& v );
	bspVertex_t& operator=( bspVertex_t v );
};
*/



struct bspTexture_t
{
    char    name[ 64 ];
    int     surfaceFlags;
    int     contentsFlags;
};

struct bspMeshVertex_t
{
    int offset;
};

struct bspEffect_t
{
    char    name[ 64 ];
    int     brush;
    int     unknown;
};

struct bspFace_t
{
    int texture;
    int effect;
    int type;

    int vertexOffset;
    int numVertexes;

    int meshVertexOffset;
    int numMeshVertexes;

    int lightmapIndex;
    int lightmapStartCorner[ 2 ];
    int lightmapSize[ 2 ];

    glm::vec3 lightmapOrigin; // in world space
    glm::vec3 lightmapStVecs[ 2 ]; // world space s/t unit vectors
    glm::vec3 normal;

    int     size[ 2 ];
};

struct bspLightmap_t
{
	byte map[ BSP_LIGHTMAP_WIDTH ][ BSP_LIGHTMAP_HEIGHT ][ 3 ]; // lightmap color data. RGB.
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

    byte*   bitsets;
};

// --------------------------------------------------------

struct triangle_t
{
	GLuint indices[ 3 ];
};

struct leafModel_t
{
	std::vector< int > modelIndices;
};
