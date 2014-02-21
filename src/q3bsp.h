#pragma once

#include "common.h"

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
    vec3f       normal;
    float       distance;
};

struct bspNode_t
{
    int     plane;
    int     children[ 2 ];

    vec3i   boxMin;
    vec3i   boxMax;
};

struct bspLeaf_t
{
    int clusterIndex;
    int areaPortal;

    vec3i   boxMin;
    vec3i   boxMax;

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
    vec3f boxMax;
    vec3f boxMin;

    int faceOffset;
    int numFaces;

    int brushOffset;
    int numBrushes;
};

struct bspVertex_t
{
    vec3f position;
    vec2f texCoord;
    vec2f lightmapCoord;
    vec3f normal;

    byte color[ 4 ];
};

struct bspTexture_t
{
    char    filename[ 64 ];
    int     surfaceFlags;
    int     contentsFlags;
};

struct bspMeshVertex_t
{
    int offset;
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

    vec3f lightmapOrigin; // in world space
    vec3f lightmapStVecs[ 2 ]; // world space s/t unit vectors
    vec3f normal;

    int     size[ 2 ];
};

struct bspVisdata_t
{
    int     numVectors;
    int     sizeVector;

    byte*   bitsets;
};

/*
=====================================================

        Q3BspParser: map loader and parser.

=====================================================
*/

class Q3BspMap
{
public:

    Q3BspMap( void );

    ~Q3BspMap( void );

    void                Read( const std::string& filepath );

    void                GenTextures( const std::string& filepath );

    void                SetVertexColorIf( bool ( predicate )( unsigned char* ), const glm::u8vec3& rgbColor );

    GLuint              GetApiTexture( int index ) const;

    bspLeaf_t*          FindClosestLeaf( const glm::vec3& camPos );

    bool                IsClusterVisible( int sourceCluster, int testCluster );

    bool                IsAllocated( void ) const { return mapAllocated; }

    void                DestroyMap( void );

    bspEntity_t         entities;

    bspNode_t*          nodes;
    bspLeaf_t*          leaves;
    bspPlane_t*         planes;

    bspVertex_t*        vertexes;
    bspTexture_t*       textures;
    bspModel_t*         models;
    bspFace_t*          faces;

    bspLeafFace_t*      leafFaces;
    bspMeshVertex_t*    meshVertexes;

    bspVisdata_t*       visdata;

    int                 entityStringLen;

    int                 numNodes;
    int                 numLeaves;
    int                 numPlanes;

    int                 numVertexes;
    int                 numTextures;
    int                 numModels;
    int                 numFaces;

    int                 numLeafFaces;
    int                 numMeshVertexes;

    int                 numVisdataVecs;

private:

    Q3BspMap( const Q3BspMap& ) = delete;
    Q3BspMap& operator=( Q3BspMap ) = delete;

    GLuint*         apiTextures;

    bspHeader_t     header;

    bool            mapAllocated;
};

INLINE GLuint Q3BspMap::GetApiTexture( int index ) const
{
    assert( mapAllocated );
    assert( index < numTextures );

    return apiTextures[ index ];
}
