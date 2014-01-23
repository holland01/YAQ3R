#include "q3m.h"
#include "log.h"

// Convert quake 3 coordinate system to a
// right handed coordinate system;
// v[ 0 ] => x
// v[ 1 ] => y
// v[ 2 ] => z
static void swizzleCoords( vec3f& v )
{
    float tmp = v.y;

    v.y = v.z;
    v.z = -tmp; // we have to invert the y-axis to accomodate the right-handed positive z facing backward.
}

// Straight outta copypasta
static void swizzleCoords( vec3i& v )
{
    int tmp = v.y;

    v.y = v.z;
    v.z = -tmp;
}

Quake3Map::Quake3Map( void )
     : mNodes( NULL ),
       mLeaves( NULL ),
       mPlanes( NULL ),
       mVertexes( NULL ),
       mModels( NULL ),
       mFaces( NULL ),
       mMapAllocd( false )
{
}

Quake3Map::~Quake3Map( void )
{
    if ( mMapAllocd )
    {
        free( mNodes );
        free( mLeaves );
        free( mPlanes );

        free( mFaces );
        free( mModels );
        free( mVertexes );
        free( mLeafFaces );
        free( mMeshVertexes );

        free( mVisData );
    }
}

void Quake3Map::read( const std::string& filepath, int divisionScale )
{
    FILE* file = fopen( filepath.c_str(), "rb" );

    if ( !file )
    {
        printf("Failed to open %s\n", filepath.c_str() );
        return;
    }

    fread( &mHeader, sizeof( BspHeader ), 1, file );

    if ( mHeader.id[ 0 ] != 'I' || mHeader.id[ 1 ] != 'B' || mHeader.id[ 2 ] != 'S' || mHeader.id[ 3 ] != 'P' )
    {
        ERROR( "Header ID does NOT match \'IBSP\'. ID read is: %s \n", mHeader.id );
    }

    if ( mHeader.version != BSP_Q3_VERSION )
    {
        ERROR( "Header version does NOT match %i. Version found is %i\n", BSP_Q3_VERSION, mHeader.version );
    }

    mNodes = ( BspNode* )malloc( mHeader.sizeNodes );
    mTotalNodes = mHeader.sizeNodes / sizeof( BspNode );
    fseek( file, mHeader.offsetNodes, SEEK_SET );
    fread( mNodes, mHeader.sizeNodes, 1, file );

    for ( int i = 0; i < mTotalNodes; ++i )
    {
        mNodes[ i ].boxMax.x /= divisionScale;
        mNodes[ i ].boxMax.y /= divisionScale;
        mNodes[ i ].boxMax.z /= divisionScale;

        mNodes[ i ].boxMin.x /= divisionScale;
        mNodes[ i ].boxMin.y /= divisionScale;
        mNodes[ i ].boxMin.z /= divisionScale;

        swizzleCoords( mNodes[ i ].boxMax );
        swizzleCoords( mNodes[ i ].boxMin );
    }

    mLeaves = ( BspLeaf* )malloc( mHeader.sizeLeaves );
    mTotalLeaves = mHeader.sizeLeaves / sizeof( BspLeaf );
    fseek( file, mHeader.offsetLeaves, SEEK_SET );
    fread( mLeaves, mHeader.sizeLeaves, 1, file );

    for ( int i = 0; i < mTotalLeaves; ++i )
    {
        mLeaves[ i ].boxMax.x /= divisionScale;
        mLeaves[ i ].boxMax.y /= divisionScale;
        mLeaves[ i ].boxMax.z /= divisionScale;

        mLeaves[ i ].boxMin.x /= divisionScale;
        mLeaves[ i ].boxMin.y /= divisionScale;
        mLeaves[ i ].boxMin.z /= divisionScale;

        swizzleCoords( mLeaves[ i ].boxMax );
        swizzleCoords( mLeaves[ i ].boxMin );
    }

    mPlanes = ( BspPlane* )malloc( mHeader.sizePlanes );
    mTotalPlanes = mHeader.sizePlanes / sizeof( BspPlane );
    fseek( file, mHeader.offsetPlanes, SEEK_SET );
    fread( mPlanes, mHeader.sizePlanes, 1, file );

    for ( int i = 0; i < mTotalPlanes; ++i )
    {
        mPlanes[ i ].normal.x /= ( float ) divisionScale;
        mPlanes[ i ].normal.y /= ( float ) divisionScale;
        mPlanes[ i ].normal.z /= ( float ) divisionScale;

        mPlanes[ i ].distance /= ( float ) divisionScale;

        swizzleCoords( mPlanes[ i ].normal );
    }

    mVertexes = ( BspVertex* )malloc( mHeader.sizeVertexes );
    mTotalVertexes = mHeader.sizeVertexes / sizeof( BspVertex );
    fseek( file, mHeader.offsetVertexes, SEEK_SET );
    fread( mVertexes, mHeader.sizeVertexes, 1, file );

    for ( int i = 0; i < mTotalVertexes; ++i )
    {
        /*
        mVertexes[ i ].position.x /= ( float ) divisionScale;
        mVertexes[ i ].position.y /= ( float ) divisionScale;
        mVertexes[ i ].position.z /= ( float ) divisionScale;

        mVertexes[ i ].normal.x /= ( float ) divisionScale;
        mVertexes[ i ].normal.y /= ( float ) divisionScale;
        mVertexes[ i ].normal.z /= ( float ) divisionScale;
        */

        swizzleCoords( mVertexes[ i ].position );
        swizzleCoords( mVertexes[ i ].normal );
    }

    mModels = ( BspModel* )malloc( mHeader.sizeModels );
    mTotalModels = mHeader.sizeModels / sizeof( BspModel );
    fseek( file, mHeader.offsetModels, SEEK_SET );
    fread( mModels, mHeader.sizeModels, 1, file );

    for ( int i = 0; i < mTotalModels; ++i )
    {
        mModels[ i ].boxMax.x /= ( float ) divisionScale;
        mModels[ i ].boxMax.y /= ( float ) divisionScale;
        mModels[ i ].boxMax.z /= ( float ) divisionScale;

        mModels[ i ].boxMin.x /= ( float ) divisionScale;
        mModels[ i ].boxMin.y /= ( float ) divisionScale;
        mModels[ i ].boxMin.z /= ( float ) divisionScale;

        swizzleCoords( mModels[ i ].boxMax );
        swizzleCoords( mModels[ i ].boxMin );
    }

    mFaces = ( BspFace* )malloc( mHeader.sizeFaces );
    mTotalFaces = mHeader.sizeFaces / sizeof( BspFace );
    fseek( file, mHeader.offsetFaces, SEEK_SET );
    fread( mFaces, mHeader.sizeFaces, 1, file );

    for ( int i = 0; i < mTotalFaces; ++i )
    {
        mFaces[ i ].normal.x /= ( float ) divisionScale;
        mFaces[ i ].normal.y /= ( float ) divisionScale;
        mFaces[ i ].normal.z /= ( float ) divisionScale;

        mFaces[ i ].lightmapOrigin.x /= ( float ) divisionScale;
        mFaces[ i ].lightmapOrigin.y /= ( float ) divisionScale;
        mFaces[ i ].lightmapOrigin.z /= ( float ) divisionScale;

        mFaces[ i ].lightmapStVecs[ 0 ].x /= ( float ) divisionScale;
        mFaces[ i ].lightmapStVecs[ 0 ].y /= ( float ) divisionScale;
        mFaces[ i ].lightmapStVecs[ 0 ].z /= ( float ) divisionScale;

        mFaces[ i ].lightmapStVecs[ 1 ].x /= ( float ) divisionScale;
        mFaces[ i ].lightmapStVecs[ 1 ].y /= ( float ) divisionScale;
        mFaces[ i ].lightmapStVecs[ 1 ].z /= ( float ) divisionScale;

        swizzleCoords( mFaces[ i ].normal );
        swizzleCoords( mFaces[ i ].lightmapOrigin );
        swizzleCoords( mFaces[ i ].lightmapStVecs[ 0 ] );
        swizzleCoords( mFaces[ i ].lightmapStVecs[ 1 ] );
    }

    mLeafFaces = ( BspLeafFace* )malloc( mHeader.sizeLeafFaces );
    mTotalLeafFaces = mHeader.sizeLeafFaces / sizeof( BspLeafFace );
    fseek( file, mHeader.offsetLeafFaces, SEEK_SET );
    fread( mLeafFaces, mHeader.sizeLeafFaces, 1, file );

    mMeshVertexes = ( BspMeshVertex* )malloc( mHeader.sizeMeshVertexes );
    mTotalMeshVertexes = mHeader.sizeMeshVertexes / sizeof( BspMeshVertex );
    fseek( file, mHeader.offsetMeshVertexes, SEEK_SET );
    fread( mMeshVertexes, mHeader.sizeMeshVertexes, 1, file );

    mVisData = ( BspVisData* )malloc( mHeader.sizeVisData );
    mTotalVisVecs = mHeader.sizeVisData;
    fseek( file, mHeader.offsetVisData, SEEK_SET );
    fread( mVisData, 2, sizeof( int ), file );
    int size = mVisData->numVectors * mVisData->sizeVector;
    mVisData->bitsets = ( byte* )malloc( size );
    fread( mVisData->bitsets, 1, size, file );

    mMapAllocd = true;

    fclose( file );

    logBspData( BSP_LOG_VERTEXES, ( void* ) mVertexes, mTotalVertexes );
    logBspData( BSP_LOG_MESH_VERTEXES, ( void* ) mMeshVertexes, mTotalMeshVertexes );
}

BspLeaf* Quake3Map::findClosestLeaf( const glm::vec3& camPos )
{
    int nodeIndex = 0;

    while ( nodeIndex >= 0 )
    {
        const BspNode* const node = mNodes + nodeIndex;
        const BspPlane* const plane = mPlanes + node->plane;

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

    return &mLeaves[ nodeIndex ];
}

bool Quake3Map::isClusterVisible( int visCluster, int testCluster )
{
    if ( !mVisData->bitsets || ( visCluster < 0 ) )
    {
        return true;
    }

    int i = ( visCluster * mVisData->sizeVector ) + ( testCluster >> 3 );

    byte visSet = mVisData->bitsets[ i ];

    return ( visSet & ( 1 << ( testCluster & 7 ) ) ) != 0;
}
