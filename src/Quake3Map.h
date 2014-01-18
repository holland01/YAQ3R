#pragma once

#include "common.h"

enum
{
    BSP_HEX_ID = 0x49425350,
    BSP_NUM_ENTRIES = 17,
    BSP_Q3_VERSION = 0x2E,

    BSP_LUMP_ENTITY = 0x00,
    BSP_LUMP_TEXTURE = 0x01,
    BSP_LUMP_PLANE = 0x02,
    BSP_LUMP_NODE = 0x03,
    BSP_LUMP_LEAVES = 0x04,
    BSP_LUMP_LEAF_FACES = 0x05,
    BSP_LUMP_LEAF_BRUSHES = 0x06,
    BSP_LUMP_MODELS = 0x07,
    BSP_LUMP_BRUSHES = 0x08,
    BSP_LUMP_BRUSH_SIDES = 0x09,
    BSP_LUMP_VERTEXES = 0x0A,
    BSP_LUMP_MESHVERTS = 0x0B,
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
    int         offset;
    int         numBytes;
};

struct BspHeader
{
    char        id[ 4 ];
    int         version;
    BspLump     lumpEntries[ BSP_NUM_ENTRIES ];
};

struct BspPlane
{
    float       normal[ 3 ];
    float       distance;
};

struct BspNode
{
    int plane;
    int children[ 2 ];
    int boxMin[ 3 ];    // bounding box int coords
    int boxMax[ 3 ];
};

struct BspLeaf
{
    int clusterIndex;
    int areaPortal;

    int boxMin[ 3 ];
    int boxMax[ 3 ];

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
    float boxMax[ 3 ];
    float boxMin[ 3 ];

    int faceOffset;
    int numFaces;

    int brushOffset;
    int numBrushes;
};

struct BspVertex
{
    float position[ 3 ];
    float texcoord[ 3 ];
    float normal[ 3 ];

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
    int numVertices;

    int meshVertexOffset;
    int numMeshVertices;

    int lightmapIndex;
    int lightmapStartCorner[ 2 ];
    int lightmapSize[ 2 ];

    float lightmapOrigin[ 3 ]; // in world space
    float lightmapStVecs[ 2 ][ 3 ]; // world space s/t unit vectors
    float normal[ 3 ];

    int size[ 2 ];
};

struct BspVisdata
{
    int     numClusters;
    int     bytesPerCluster;

    byte*   bitsets;
};

class Quake3Map
{
public:

    Quake3Map( void );

    ~Quake3Map( void );

    void read( const std::string& filepath );

    BspLeaf* findClosestLeaf( const QVector3D& camPos );

    bool isClusterVisible( int visCluster, int testCluster );

    void loadVertexBuffer( std::vector< QVector3D >& vertices, QGLBuffer& vertexBuffer );

    void loadIndexBuffer( std::vector< int >& indices, QGLBuffer& indexBuffer );

    BspNode*        mNodeBuffer;
    BspLeaf*        mLeafBuffer;
    BspPlane*       mPlaneBuffer;
    BspVertex*      mVertexBuffer;
    BspModel*       mModelBuffer;
    BspFace*        mFaceBuffer;
    BspLeafFace*    mLeafFaces;
    BspMeshVertex*  mMeshVertices;
    BspVisdata*     mVisdata;

    size_t          mTotalNodes;
    size_t          mTotalLeaves;
    size_t          mTotalPlanes;
    size_t          mTotalVertices;
    size_t          mTotalModels;
    size_t          mTotalFaces;
    size_t          mTotalLeafFaces;
    size_t          mTotalMeshVerts;

    size_t          mVisdataSize;

private:

    void convertFaceRangeToRHC( size_t start, size_t end );

    byte*           mDataBuffer;

    BspHeader*      mHeader;
};
