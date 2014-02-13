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
    BSP_FACE_TYPE_BILLBOARD = 0x4
};

/*
=====================================================

                BSP Map Structs

=====================================================
*/

struct BSPEntity
{
    char* infoString;
};

struct BSPLump
{
    int offset;
    int length;
};

struct BSPHeader
{
    char        id[ 4 ];

    int         version;

    BSPLump     directories[ 17 ];
};

struct BSPPlane
{
    vec3f       normal;
    float       distance;
};

struct BSPNode
{
    int     plane;
    int     children[ 2 ];

    vec3i   boxMin;
    vec3i   boxMax;
};

struct BSPLeaf
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

struct BSPLeafFace
{
    int index;
};

struct BSPModel
{
    vec3f boxMax;
    vec3f boxMin;

    int faceOffset;
    int numFaces;

    int brushOffset;
    int numBrushes;
};

struct BSPVertex
{
    vec3f position;
    vec2f texcoord;
    vec2f lightmapCoord;
    vec3f normal;

    byte color[ 4 ];
};

struct BSPMeshVertex
{
    int offset;
};

struct BSPFace
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

struct BSPVisdata
{
    int     numVectors;
    int     sizeVector;

    byte*   bitsets;
};

/*
=====================================================

        Quake3Map: map loader and parser.

=====================================================
*/

class Quake3Map
{
public:

    Quake3Map( void );

    ~Quake3Map( void );

    void            Read( const std::string& filepath );

    void            SetVertexColorIf( bool ( predicate )( unsigned char* ), const glm::u8vec3& rgbColor );

    BSPLeaf*        FindClosestLeaf( const glm::vec3& camPos );

    bool            IsClusterVisible( int sourceCluster, int testCluster );

    bool            IsAllocated( void ) const { return mapAllocated; }

    void            DestroyMap( void );

    BSPEntity       entities;

    BSPNode*        nodes;
    BSPLeaf*        leaves;
    BSPPlane*       planes;
    BSPVertex*      vertexes;
    BSPModel*       models;
    BSPFace*        faces;
    BSPLeafFace*    leafFaces;
    BSPMeshVertex*  meshVertexes;
    BSPVisdata*     visdata;

    int             entityStringLen;

    int             numNodes;
    int             numLeaves;
    int             numPlanes;
    int             numVertexes;
    int             numModels;
    int             numFaces;
    int             numLeafFaces;
    int             numMeshVertexes;
    int             numVisdataVecs;

private:

    BSPHeader       header;

    bool            mapAllocated;
};
