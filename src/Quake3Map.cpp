#include "Quake3Map.h"
#include <QErrorMessage>
#include <QFile>

// Convert quake 3 coordinate system to a
// right handed coordinate system;
// v[ 0 ] => x
// v[ 1 ] => y
// v[ 2 ] => z
static void q3cToRightHand( float v[ 3 ] )
{
    float tmp = v[ 1 ];

    v[ 1 ] = v[ 2 ];
    v[ 2 ] = -tmp; // we have to invert the y-axis to accomodate the right-handed positive z facing backward.
}

// Straight outta copypasta
static void q3cToRightHand( int v[ 3 ] )
{
    int tmp = v[ 1 ];

    v[ 1 ] = v[ 2 ];
    v[ 2 ] = -tmp;
}

Quake3Map::Quake3Map( void )
     : mNodeBuffer( NULL ),
       mLeafBuffer( NULL ),
       mPlaneBuffer( NULL ),
       mVertexBuffer( NULL ),
       mModelBuffer( NULL ),
       mFaceBuffer( NULL ),

       mDataBuffer( NULL ),
       mHeader( NULL )
{
}

Quake3Map::~Quake3Map( void )
{
    delete[] mDataBuffer;
}

void Quake3Map::read( const std::string& filepath )
{
    {
        FILE* mapFile = fopen( filepath.c_str(), "rb" );

        if ( !mapFile )
        {
            qWarning() << "Failed to open " << filepath.c_str();
            return;
        }

        fseek( mapFile, 0, SEEK_END );
        long size = ftell( mapFile );
        fseek( mapFile, 0, SEEK_SET );

        mDataBuffer = new byte[ size ];

        fread( mDataBuffer, sizeof( byte ), size, mapFile );
        fclose( mapFile );
    }

    mHeader = ( BspHeader* ) mDataBuffer;

    if ( mHeader->id[ 0 ] != 'I' || mHeader->id[ 1 ] != 'B' || mHeader->id[ 2 ] != 'S' || mHeader->id[ 3 ] != 'P' )
    {
        qWarning() << "Header ID does NOT match \'IBSP\'. ID read is: " << mHeader->id;
        return;
    }

    if ( mHeader->version != BSP_Q3_VERSION )
    {
        qWarning() << "Header version does NOT match " << BSP_Q3_VERSION << ". Version found is " << mHeader->version;
        return;
    }

    mNodeBuffer = ( BspNode* )( mDataBuffer + mHeader->lumpEntries[ BSP_LUMP_NODE ].offset );
    mLeafBuffer = ( BspLeaf* )( mDataBuffer + mHeader->lumpEntries[ BSP_LUMP_LEAVES ].offset );
    mPlaneBuffer = ( BspPlane* )( mDataBuffer + mHeader->lumpEntries[ BSP_LUMP_PLANE ].offset );
    mVertexBuffer = ( BspVertex* )( mDataBuffer + mHeader->lumpEntries[ BSP_LUMP_VERTEXES ].offset );
    mModelBuffer = ( BspModel* )( mDataBuffer + mHeader->lumpEntries[ BSP_LUMP_MODELS ].offset );
    mFaceBuffer = ( BspFace* )( mDataBuffer + mHeader->lumpEntries[ BSP_LUMP_FACES ].offset );
    mLeafFaces  = ( BspLeafFace* )( mDataBuffer + mHeader->lumpEntries[ BSP_LUMP_LEAF_FACES ].offset );
    mMeshVertices = ( BspMeshVertex* )( mDataBuffer + mHeader->lumpEntries[ BSP_LUMP_MESHVERTS ].offset );
    mVisdata = ( BspVisdata* )( mDataBuffer + mHeader->lumpEntries[ BSP_LUMP_VISDATA ].offset );

    mTotalNodes = mHeader->lumpEntries[ BSP_LUMP_NODE ].numBytes / sizeof( BspNode );
    mTotalLeaves = mHeader->lumpEntries[ BSP_LUMP_NODE ].numBytes / sizeof( BspLeaf );
    mTotalPlanes = mHeader->lumpEntries[ BSP_LUMP_PLANE ].numBytes / sizeof( BspPlane );
    mTotalVertices = mHeader->lumpEntries[ BSP_LUMP_VERTEXES ].numBytes / sizeof( BspVertex );
    mTotalModels = mHeader->lumpEntries[ BSP_LUMP_MODELS ].numBytes / sizeof( BspModel );
    mTotalFaces = mHeader->lumpEntries[ BSP_LUMP_FACES ].numBytes / sizeof( BspFace );
    mTotalLeafFaces = mHeader->lumpEntries[ BSP_LUMP_LEAF_FACES ].numBytes / sizeof( BspLeafFace );
    mTotalMeshVerts = mHeader->lumpEntries[ BSP_LUMP_MESHVERTS ].numBytes / sizeof( BspMeshVertex );
    mVisdataSize = mHeader->lumpEntries[ BSP_LUMP_VISDATA ].numBytes;


    // Perform coordinate space conversions
    //=============================================

    const int NODES = 0;
    const int LEAVES = 1;
    const int FACES = 2;
    const int PLANES = 3;
    const int MODELS = 5;
    const int VERTICES = 6;

    size_t iterators[ 6 ];
    memset( iterators, 0, sizeof( iterators ) );

    do
    {
        // Nodes
        if ( iterators[ NODES ] < mTotalNodes )
        {
            q3cToRightHand( mNodeBuffer[ iterators[ NODES ] ].boxMax );
            q3cToRightHand( mNodeBuffer[ iterators[ NODES ] ].boxMin );

            iterators[ NODES ]++;
        }

        // Leaves
        if ( iterators[ LEAVES ] < mTotalLeaves )
        {
            q3cToRightHand( mLeafBuffer[ iterators[ LEAVES ] ].boxMax );
            q3cToRightHand( mLeafBuffer[ iterators[ LEAVES ] ].boxMin );

            iterators[ LEAVES ]++;
        }

        // Faces
        if ( iterators[ FACES ] < mTotalFaces )
        {
            q3cToRightHand( mFaceBuffer[ iterators[ FACES ] ].normal );

            iterators[ FACES ]++;
        }

        // Planes
        if ( iterators[ PLANES ] < mTotalPlanes )
        {
            q3cToRightHand( mPlaneBuffer[ iterators[ PLANES ] ].normal );

            iterators[ PLANES ]++;
        }

        // Models
        if ( iterators[ MODELS ] < mTotalModels )
        {
            q3cToRightHand( mModelBuffer[ iterators[ MODELS ] ].boxMax );
            q3cToRightHand( mModelBuffer[ iterators[ MODELS ] ].boxMin );

            iterators[ MODELS ]++;
        }

        // Vertices
        if ( iterators[ VERTICES ] < mTotalVertices )
        {
            q3cToRightHand( mVertexBuffer[ iterators[ VERTICES ] ].normal );
            q3cToRightHand( mVertexBuffer[ iterators[ VERTICES ] ].position );

            iterators[ VERTICES ]++;
        }

        // It's not pretty, but it works.
    }
    while(  iterators[ NODES ] < mTotalNodes
        &&  iterators[ LEAVES ] < mTotalLeaves
        &&  iterators[ FACES ] < mTotalFaces
        &&  iterators[ PLANES ] < mTotalPlanes
        &&  iterators[ MODELS ] < mTotalModels
        &&  iterators[ VERTICES ] < mTotalVertices );
}

void Quake3Map::loadVertexBuffer( QGLBuffer& vertexBuffer )
{
    vertexBuffer.allocate( ( void* ) mVertexBuffer, mTotalVertices );
    vertexBuffer.setUsagePattern( QGLBuffer::DynamicDraw );
}

void Quake3Map::loadIndexBuffer( QGLBuffer& indexBuffer )
{
    std::vector< int > indices;

    for ( size_t i = 0; i < mTotalFaces; ++i )
    {
        BspFace* face = mFaceBuffer + i;

        size_t end = face->numVertices;

        for ( size_t j = face->vertexOffset; j < end; ++j )
        {
            indices.push_back( j );
        }
    }

    indexBuffer.allocate( &indices[ 0 ], indices.size() * sizeof( int ) );
    indexBuffer.setUsagePattern( QGLBuffer::DynamicDraw );
}

BspLeaf* Quake3Map::findClosestLeaf( const QVector3D& camPos )
{
    int nodeIndex = 0;

    while ( nodeIndex >= 0 )
    {
        const BspNode* const node = mNodeBuffer + nodeIndex;
        const BspPlane* const plane = mPlaneBuffer + node->plane;

        // If the distance from the camera to the plane is >= 0,
        // then our needed camera data is in a leaf somewhere in front of this node,
        // otherwise it's behind the node somewhere.

        QVector3D planeNormal( plane->normal[ 0 ], plane->normal[ 1 ], plane->normal[ 2 ] );

        float distance = QVector3D::dotProduct( planeNormal, camPos ) - plane->distance;

        if ( distance >= 0 )
            nodeIndex = node->children[ 0 ];
        else
            nodeIndex = node->children[ 1 ];
    }

    nodeIndex = -( nodeIndex + 1 );

    return &mLeafBuffer[ nodeIndex ];
}

bool Quake3Map::isClusterVisible( int visCluster, int testCluster )
{
    if ( mVisdata->bitsets == NULL || ( visCluster < 0 ) )
    {
        return true;
    }

    int i = ( visCluster * mVisdata->bytesPerCluster ) + ( testCluster >> 3 );

    byte visSet = mVisdata->bitsets[ i ];

    return ( visSet & ( 1 << ( testCluster & 7 ) ) ) != 0;
}
