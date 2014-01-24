#pragma once

#include "common.h"

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
    BSP_FACE_TYPE_BILLBOARD = 0x4

};

struct BspLump
{
    int offset;
    int length;
};

struct BspHeader
{
    char        id[ 4 ];

    int         version;

    BspLump     directories[ 17 ];
};


struct BspPlane
{
    vec3f       normal;
    float       distance;
};

struct BspNode
{
    int     plane;
    int     children[ 2 ];

    vec3i   boxMin;
    vec3i   boxMax;
};

struct BspLeaf
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

struct BspLeafFace
{
    int index;
};

struct BspModel
{
    vec3f boxMax;
    vec3f boxMin;

    int faceOffset;
    int numFaces;

    int brushOffset;
    int numBrushes;
};

struct BspVertex
{
    vec3f position;
    vec2f texcoords[ 2 ];
    vec3f normal;

    byte color[ 4 ];
};

struct BspMeshVertex
{
    int offset;
};

struct BspFace
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

struct BspVisData
{
    int     numVectors;
    int     sizeVector;

    byte*   bitsets;
};

class Quake3Map
{
public:

    Quake3Map( void );

    ~Quake3Map( void );

    void read( const std::string& filepath, int divisionScale );

    BspLeaf* findClosestLeaf( const glm::vec3& camPos );

    bool isClusterVisible( int visCluster, int testCluster );

    BspNode*        mNodes;
    BspLeaf*        mLeaves;
    BspPlane*       mPlanes;
    BspVertex*      mVertexes;
    BspModel*       mModels;
    BspFace*        mFaces;
    BspLeafFace*    mLeafFaces;
    BspMeshVertex*  mMeshVertexes;
    BspVisData*     mVisData;

    int          mTotalNodes;
    int          mTotalLeaves;
    int          mTotalPlanes;
    int          mTotalVertexes;
    int          mTotalModels;
    int          mTotalFaces;
    int          mTotalLeafFaces;
    int          mTotalMeshVertexes;
    int          mTotalVisVecs;

private:

    void convertFaceRangeToRHC( size_t start, size_t end );

    BspHeader       mHeader;

    bool            mMapAllocd;
};
