#include "q3m.h"
#include "log.h"
#include "mtrand.h"

/*
=====================================================

SwizzleCoords

Convert 3D vector formatted in Quake 3 coordinate system to a
right-handed coordinate system 3D vector.

original:       { x => -left/+right, y => -backward/+forward, z => -down/+up }
right-handed:   { x => -left/+right, y => -down/+up,          z => +backward/-forward }

=====================================================
*/

static void SwizzleCoords( vec3f& v )
{
    float tmp = v.y;

    v.y = v.z;
    v.z = -tmp;
}

// Straight outta copypasta ( for integer vectors )
static void SwizzleCoords( vec3i& v )
{
    int tmp = v.y;

    v.y = v.z;
    v.z = -tmp;
}

/*
=====================================================

Quake3Map::Quake3Map

=====================================================
*/

Quake3Map::Quake3Map( void )
     : nodes( NULL ),
       leaves( NULL ),
       planes( NULL ),
       vertexes( NULL ),
       models( NULL ),
       faces( NULL ),
       mapAllocated( false )
{
    entities.infoString = NULL;
}

/*
=====================================================

Quake3Map::~Quake3Map

=====================================================
*/

Quake3Map::~Quake3Map( void )
{
    DestroyMap();
}

/*
=====================================================

Quake3Map::DestroyMap

Free all dynamically allocated data

=====================================================
*/

void Quake3Map::DestroyMap( void )
{
    if ( mapAllocated )
    {
        free( nodes );
        free( leaves );
        free( planes );

        free( faces );
        free( models );
        free( vertexes );
        free( leafFaces );
        free( meshVertexes );
        free( entities.infoString );

        free( visdata );

        numFaces = 0;
        numLeafFaces = 0;
        numLeaves = 0;
        numMeshVertexes = 0;
        numModels = 0;
        numNodes = 0;
        numPlanes = 0;
        numVertexes = 0;
        numVisdataVecs = 0;

        mapAllocated = false;
    }
}

/*
=====================================================

Quake3Map::Read

            read all map data into its respective
            buffer, defined by structs.

            "divisionScale" param is used to pre-divide
            vertex, normals, and bounding box data
            in the map by a specified size.

=====================================================
*/

void Quake3Map::Read( const std::string& filepath, int divisionScale )
{
    if ( divisionScale <= 0 )
    {
        divisionScale = 1;
    }

    FILE* file = fopen( filepath.c_str(), "rb" );

    if ( !file )
    {
        printf("Failed to open %s\n", filepath.c_str() );
        return;
    }

    fread( &header, sizeof( BSPHeader ), 1, file );

    if ( header.id[ 0 ] != 'I' || header.id[ 1 ] != 'B' || header.id[ 2 ] != 'S' || header.id[ 3 ] != 'P' )
    {
        ERROR( "Header ID does NOT match \'IBSP\'. ID read is: %s \n", header.id );
    }

    if ( header.version != BSP_Q3_VERSION )
    {
        ERROR( "Header version does NOT match %i. Version found is %i\n", BSP_Q3_VERSION, header.version );
    }
    /*

    entities.infoString = ( char* ) malloc( header.directories[ BSP_LUMP_ENTITIES ].length );
    entityStringLen = header.directories[ BSP_LUMP_ENTITIES ].length / sizeof( char );
    fseek( file, header.directories[ BSP_LUMP_ENTITIES ].offset, SEEK_SET );
    fread( entities.infoString, header.directories[ BSP_LUMP_ENTITIES ].length, 1, file );
*/
    nodes = ( BSPNode* )malloc( header.directories[ BSP_LUMP_NODES ].length );
    numNodes = header.directories[ BSP_LUMP_NODES ].length / sizeof( BSPNode );
    fseek( file, header.directories[ BSP_LUMP_NODES ].offset, SEEK_SET );
    fread( nodes, header.directories[ BSP_LUMP_NODES ].length, 1, file );

    for ( int i = 0; i < numNodes; ++i )
    {
        nodes[ i ].boxMax.x /= divisionScale;
        nodes[ i ].boxMax.y /= divisionScale;
        nodes[ i ].boxMax.z /= divisionScale;

        nodes[ i ].boxMin.x /= divisionScale;
        nodes[ i ].boxMin.y /= divisionScale;
        nodes[ i ].boxMin.z /= divisionScale;

        SwizzleCoords( nodes[ i ].boxMax );
        SwizzleCoords( nodes[ i ].boxMin );
    }

    leaves = ( BSPLeaf* )malloc( header.directories[ BSP_LUMP_LEAVES ].length );
    numLeaves = header.directories[ BSP_LUMP_LEAVES ].length / sizeof( BSPLeaf );
    fseek( file, header.directories[ BSP_LUMP_LEAVES ].offset, SEEK_SET );
    fread( leaves, header.directories[ BSP_LUMP_LEAVES ].length, 1, file );

    for ( int i = 0; i < numLeaves; ++i )
    {
        leaves[ i ].boxMax.x /= divisionScale;
        leaves[ i ].boxMax.y /= divisionScale;
        leaves[ i ].boxMax.z /= divisionScale;

        leaves[ i ].boxMin.x /= divisionScale;
        leaves[ i ].boxMin.y /= divisionScale;
        leaves[ i ].boxMin.z /= divisionScale;

        SwizzleCoords( leaves[ i ].boxMax );
        SwizzleCoords( leaves[ i ].boxMin );
    }

    planes = ( BSPPlane* )malloc( header.directories[ BSP_LUMP_PLANES ].length );
    numPlanes = header.directories[ BSP_LUMP_PLANES ].length / sizeof( BSPPlane );
    fseek( file, header.directories[ BSP_LUMP_PLANES ].offset, SEEK_SET );
    fread( planes, header.directories[ BSP_LUMP_PLANES ].length, 1, file );

    for ( int i = 0; i < numPlanes; ++i )
    {
        planes[ i ].normal.x /= ( float ) divisionScale;
        planes[ i ].normal.y /= ( float ) divisionScale;
        planes[ i ].normal.z /= ( float ) divisionScale;

        planes[ i ].distance /= ( float ) divisionScale;

        SwizzleCoords( planes[ i ].normal );
    }

    vertexes = ( BSPVertex* )malloc( header.directories[ BSP_LUMP_VERTEXES ].length );
    numVertexes = header.directories[ BSP_LUMP_VERTEXES ].length / sizeof( BSPVertex );
    fseek( file, header.directories[ BSP_LUMP_VERTEXES ].offset, SEEK_SET );
    fread( vertexes, header.directories[ BSP_LUMP_VERTEXES ].length, 1, file );

    for ( int i = 0; i < numVertexes; ++i )
    {
        vertexes[ i ].position.x /= ( float ) divisionScale;
        vertexes[ i ].position.y /= ( float ) divisionScale;
        vertexes[ i ].position.z /= ( float ) divisionScale;

        vertexes[ i ].normal.x /= ( float ) divisionScale;
        vertexes[ i ].normal.y /= ( float ) divisionScale;
        vertexes[ i ].normal.z /= ( float ) divisionScale;

        vertexes[ i ].color[ 0 ] = mtrand_range( 0, 255 );
        vertexes[ i ].color[ 1 ] = mtrand_range( 0, 255 );
        vertexes[ i ].color[ 2 ] = mtrand_range( 0, 255 );
        vertexes[ i ].color[ 3 ] = mtrand_range( 0, 255 );

        SwizzleCoords( vertexes[ i ].position );
        SwizzleCoords( vertexes[ i ].normal );
    }

    models = ( BSPModel* )malloc( header.directories[ BSP_LUMP_MODELS ].length );
    numModels = header.directories[ BSP_LUMP_MODELS ].length / sizeof( BSPModel );
    fseek( file, header.directories[ BSP_LUMP_MODELS ].offset, SEEK_SET );
    fread( models, header.directories[ BSP_LUMP_MODELS ].length, 1, file );

    for ( int i = 0; i < numModels; ++i )
    {
        models[ i ].boxMax.x /= ( float ) divisionScale;
        models[ i ].boxMax.y /= ( float ) divisionScale;
        models[ i ].boxMax.z /= ( float ) divisionScale;

        models[ i ].boxMin.x /= ( float ) divisionScale;
        models[ i ].boxMin.y /= ( float ) divisionScale;
        models[ i ].boxMin.z /= ( float ) divisionScale;

        SwizzleCoords( models[ i ].boxMax );
        SwizzleCoords( models[ i ].boxMin );
    }

    faces = ( BSPFace* )malloc( header.directories[ BSP_LUMP_FACES ].length );
    numFaces = header.directories[ BSP_LUMP_FACES ].length / sizeof( BSPFace );
    fseek( file, header.directories[ BSP_LUMP_FACES ].offset, SEEK_SET );
    fread( faces, header.directories[ BSP_LUMP_FACES ].length, 1, file );

    for ( int i = 0; i < numFaces; ++i )
    {
        faces[ i ].normal.x /= ( float ) divisionScale;
        faces[ i ].normal.y /= ( float ) divisionScale;
        faces[ i ].normal.z /= ( float ) divisionScale;

        faces[ i ].lightmapOrigin.x /= ( float ) divisionScale;
        faces[ i ].lightmapOrigin.y /= ( float ) divisionScale;
        faces[ i ].lightmapOrigin.z /= ( float ) divisionScale;

        faces[ i ].lightmapStVecs[ 0 ].x /= ( float ) divisionScale;
        faces[ i ].lightmapStVecs[ 0 ].y /= ( float ) divisionScale;
        faces[ i ].lightmapStVecs[ 0 ].z /= ( float ) divisionScale;

        faces[ i ].lightmapStVecs[ 1 ].x /= ( float ) divisionScale;
        faces[ i ].lightmapStVecs[ 1 ].y /= ( float ) divisionScale;
        faces[ i ].lightmapStVecs[ 1 ].z /= ( float ) divisionScale;

        SwizzleCoords( faces[ i ].normal );
        SwizzleCoords( faces[ i ].lightmapOrigin );
        SwizzleCoords( faces[ i ].lightmapStVecs[ 0 ] );
        SwizzleCoords( faces[ i ].lightmapStVecs[ 1 ] );
    }

    leafFaces = ( BSPLeafFace* )malloc( header.directories[ BSP_LUMP_LEAF_FACES ].length );
    numLeafFaces = header.directories[ BSP_LUMP_LEAF_FACES ].length / sizeof( BSPLeafFace );
    fseek( file, header.directories[ BSP_LUMP_LEAF_FACES ].offset, SEEK_SET );
    fread( leafFaces, header.directories[ BSP_LUMP_LEAF_FACES ].length, 1, file );

    meshVertexes = ( BSPMeshVertex* )malloc( header.directories[ BSP_LUMP_MESH_VERTEXES ].length );
    numMeshVertexes = header.directories[ BSP_LUMP_MESH_VERTEXES ].length / sizeof( BSPMeshVertex );
    fseek( file, header.directories[ BSP_LUMP_MESH_VERTEXES ].offset, SEEK_SET );
    fread( meshVertexes, header.directories[ BSP_LUMP_MESH_VERTEXES ].length, 1, file );

    visdata = ( BSPVisdata* )malloc( header.directories[ BSP_LUMP_VISDATA ].length );
    numVisdataVecs = header.directories[ BSP_LUMP_VISDATA ].length;
    fseek( file, header.directories[ BSP_LUMP_VISDATA ].offset, SEEK_SET );
    fread( visdata, sizeof( int ), 2, file );
    int size = visdata->numVectors * visdata->sizeVector;
    visdata->bitsets = ( byte* )malloc( size );
    fread( visdata->bitsets, size, 1, file );

    mapAllocated = true;

    fclose( file );

   // LogBSPData( BSP_LUMP_VERTEXES, ( void* ) vertexes, numVertexes );
   // LogBSPData( BSP_LUMP_MESH_VERTEXES, ( void* ) meshVertexes, numMeshVertexes );
    //LogBSPData( BSP_LUMP_ENTITIES, ( void* ) entities.infoString, entityStringLen );
}

/*
=====================================================

Quake3Map::FindClosestLeaf

Find the closest map leaf-node to the given camera position.
The leaf-node returned is further evaluated to detect faces the
camera is looking at.

=====================================================
*/

BSPLeaf* Quake3Map::FindClosestLeaf( const glm::vec3& camPos )
{
    int nodeIndex = 0;

    while ( nodeIndex >= 0 )
    {
        const BSPNode* const node = nodes + nodeIndex;
        const BSPPlane* const plane = planes + node->plane;

        // If the distance from the camera to the plane is >= 0,
        // then our needed camera data is in a leaf somewhere in front of this node,
        // otherwise it's behind the node somewhere.

        glm::vec3 planeNormal( plane->normal.x, plane->normal.y, plane->normal.z );

        float distance = glm::dot( planeNormal, camPos ) - plane->distance;

        if ( distance >= 0 )
            nodeIndex = node->children[ 0 ];
        else
            nodeIndex = node->children[ 1 ];
    }

    nodeIndex = -( nodeIndex + 1 );

    return &leaves[ nodeIndex ];
}

/*
=====================================================

Quake3Map::IsClusterVisible

Check to see if param sourceCluster can "see" param testCluster.
The algorithm used is standard and can be derived from the documentation
found in the link posted in q3m.h.

=====================================================
*/

bool Quake3Map::IsClusterVisible( int sourceCluster, int testCluster )
{
    if ( !visdata->bitsets || ( sourceCluster < 0 ) )
    {
        return true;
    }

    int i = ( sourceCluster * visdata->sizeVector ) + ( testCluster >> 3 );

    byte visSet = visdata->bitsets[ i ];

    return ( visSet & ( 1 << ( testCluster & 7 ) ) ) != 0;
}


