#pragma once

#include "common.h"
#include "bezpatch.h"

/*
=====================================================

Author: Holland Schutte
License: WTFPL

                    q3m.h

        Contains data structure definitions
        for ID's IBSP map format, along with
        a map loader and parser.

        Readers of this file should be familiar with
        documentation on ID's BSP file format if they
        are to understand this.

        Basic documentation can be found at http://www.mralligator.com/q3/

=====================================================
*/


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
    BSP_LIGHTMAP_HEIGHT = 128
};

// Map loader-specific flags
enum 
{
	Q3LOAD_TEXTURE_SRGB = 1,
	Q3LOAD_TEXTURE_ANISOTROPY = 1 << 2,
	Q3LOAD_TEXTURE_MIPMAP = 1 << 3,
	
	Q3LOAD_ALL = Q3LOAD_TEXTURE_SRGB | Q3LOAD_TEXTURE_ANISOTROPY | Q3LOAD_TEXTURE_MIPMAP
};

/*
=====================================================

                BSP Map Structs

=====================================================
*/

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
    vec3f_t       normal;
    float       distance;
};

struct bspNode_t
{
    int     plane;
    int     children[ 2 ];

    vec3i_t   boxMin;
    vec3i_t   boxMax;
};

struct bspLeaf_t
{
    int clusterIndex;
    int areaPortal;

    vec3i_t   boxMin;
    vec3i_t   boxMax;

    int leafFaceOffset;
    int numLeafFaces;

    int leafBrushOffset;
    int numLeafBrushes;
};

struct bspLeafFace_t
{
    int index;
};

struct bspModel_t
{
    vec3f_t boxMax;
    vec3f_t boxMin;

    int faceOffset;
    int numFaces;

    int brushOffset;
    int numBrushes;
};

struct bspVertex_t
{
    vec3f_t position;
    vec2f_t texCoords[ 2 ]; // 0 => surface, 1 => lightmap
    vec3f_t normal;

    byte color[ 4 ];
};

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

    vec3f_t lightmapOrigin; // in world space
    vec3f_t lightmapStVecs[ 2 ]; // world space s/t unit vectors
    vec3f_t normal;

    int     size[ 2 ];
};

struct bspLightmap_t
{
	byte map[ BSP_LIGHTMAP_WIDTH ][ BSP_LIGHTMAP_HEIGHT ][ 3 ]; // lightmap color data. RGB.
};

struct bspLightvol_t
{
	byte ambient[ 3 ];		// RGB color
	byte directional[ 3 ];	// RGB color
	byte direction[ 2 ];	// - to light; 0 => phi, 1 => theta
};

struct bspVisdata_t
{
    int     numVectors;
    int     sizeVector;

    byte*   bitsets;
};

INLINE bspVertex_t operator +( const bspVertex_t& a, const bspVertex_t& b )
{
	bspVertex_t vert;
	
	vert.position = a.position + b.position;

	vert.color[ 0 ] = a.color[ 0 ];
	vert.color[ 1 ] = a.color[ 1 ];
	vert.color[ 2 ] = a.color[ 2 ];
	vert.color[ 3 ] = a.color[ 3 ];
	
	vert.normal = a.normal + b.normal;
	vert.texCoords[ 0 ] = a.texCoords[ 0 ] + b.texCoords[ 0 ];
    vert.texCoords[ 1 ] = a.texCoords[ 1 ] + b.texCoords[ 1 ];

	// TODO: lightmapCoords?

	return vert;
}

INLINE bspVertex_t operator *( const bspVertex_t& a, float b )
{
	bspVertex_t vert;
	
	vert.position = a.position * b;

	vert.normal = a.normal * b;
	vert.texCoords[ 0 ] = a.texCoords[ 0 ] * b;
    vert.texCoords[ 1 ] = a.texCoords[ 1 ] * b;

	memcpy( vert.color, a.color, sizeof( vert.color ) );
	
	// TODO: lightmapCoords?
	// TODO: colors with floats?

	return vert;
}

/*
=====================================================

        Q3BspParser: map loader and parser.

=====================================================
*/

struct shaderInfo_t;

struct mapModel_t
{
	std::vector< GLuint > indices;
};

struct mapData_t
{
	const char*			basePath; // root directory of the map

	byte*				buffer;  // all file memory comes from this

	bspHeader_t*		header;

	bspEntity_t         entities;

	bspEffect_t*        effectShaders;

    bspNode_t*          nodes;
    bspLeaf_t*          leaves;
    bspPlane_t*         planes;

    bspVertex_t*        vertexes;
    bspTexture_t*       textures;
    bspModel_t*         models;

	bspEffect_t*		effects;
    bspFace_t*          faces;

    bspLeafFace_t*      leafFaces;
    bspMeshVertex_t*    meshVertexes;

	bspLightmap_t*		lightmaps;
	bspLightvol_t*		lightvols;

    bspVisdata_t*       visdata;

	int                 entityStringLen;
    int                 numEffectShaders;

    int                 numNodes;
    int                 numLeaves;
    int                 numPlanes;

    int                 numVertexes;
    int                 numTextures;
    int                 numModels;

	int					numEffects;
    int                 numFaces;

    int                 numLeafFaces;
    int                 numMeshVertexes;

	int					numLightmaps;
	int					numLightvols;

    int                 numVisdataVecs;
};

class Q3BspMap
{
private:

    Q3BspMap( const Q3BspMap& ) = delete;
    Q3BspMap& operator=( Q3BspMap ) = delete;

    bool						mapAllocated;

public:

	std::vector< GLuint >		glTextures;		// has one->one map with texture indices
	std::vector< GLuint >		glSamplers;		// has one->one map with glTextures
	std::vector< mapModel_t >	glFaces;		// has one->one map with face indices
	std::vector< GLuint >		glLightmaps;	// textures - has one->one map with lightmap indices
					GLuint		glLightmapSampler;

	std::map< std::string, shaderInfo_t > effectShaders;

    Q3BspMap( void );
    ~Q3BspMap( void );

	mapData_t					data;

    void						Read( const std::string& filepath, const int scale, uint32_t loadFlags );

    void						SetVertexColorIf( bool ( predicate )( unsigned char* ), const glm::u8vec3& rgbColor );

    bspLeaf_t*					FindClosestLeaf( const glm::vec3& camPos );

    bool						IsClusterVisible( int sourceCluster, int testCluster );

    bool						IsAllocated( void ) const { return mapAllocated; }

    void						DestroyMap( void );
};