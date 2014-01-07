#include "Q3Map.h"
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
     : mDataBuffer( NULL ),
       mHeader( NULL ),
       mNodeBuffer( NULL ),
       mLeafBuffer( NULL ),
       mPlaneBuffer( NULL ),
       mVertexBuffer( NULL ),
       mModelBuffer( NULL ),
       mFaceBuffer( NULL )
{
}

Quake3Map::~Quake3Map( void )
{
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

        fread( mDataBuffer, size, 1, mapFile );
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

    mTotalNodes = mHeader->lumpEntries[ BSP_LUMP_NODE ].numBytes / sizeof( BspNode );
    mTotalLeaves = mHeader->lumpEntries[ BSP_LUMP_NODE ].numBytes / sizeof( BspLeaf );
    mTotalPlanes = mHeader->lumpEntries[ BSP_LUMP_PLANE ].numBytes / sizeof( BspPlane );
    mTotalVertices = mHeader->lumpEntries[ BSP_LUMP_VERTEXES ].numBytes / sizeof( BspVertex );
    mTotalModels = mHeader->lumpEntries[ BSP_LUMP_MODELS ].numBytes / sizeof( BspModel );
    mTotalFaces = mHeader->lumpEntries[ BSP_LUMP_FACES ].numBytes / sizeof( BspFace );
    mTotalLeafFaces = mHeader->lumpEntries[ BSP_LUMP_LEAF_FACES ].numBytes / sizeof( BspLeafFace );

    // Perform coordinate space conversions
    //=============================================

    // Nodes and Planes
    for ( size_t i = 0; i < mTotalNodes; ++i )
    {
        q3cToRightHand( mNodeBuffer[ i ].boxMax );
        q3cToRightHand( mNodeBuffer[ i ].boxMin );

        q3cToRightHand( mPlaneBuffer[ mNodeBuffer[ i ].plane ].normal );
    }

    // Leaves
    for ( size_t i = 0; i < mTotalLeaves; ++i )
    {
        BspLeaf* const pLeaf = mLeafBuffer + i;

        q3cToRightHand( pLeaf->boxMax );
        q3cToRightHand( pLeaf->boxMin );

        // Corresponding Leaf faces
        convertFaceRangeToRHC( ( size_t ) pLeaf->leafFaceOffset, ( size_t )( pLeaf->leafFaceOffset + pLeaf->numLeafFaces ) );
    }

    // Models
    for ( size_t i = 0; i < mTotalModels; ++i )
    {
        BspModel* const pMod = mModelBuffer + i;

        q3cToRightHand( pMod->boxMax );
        q3cToRightHand( pMod->boxMin );

        // Corresponding Model Faces
        convertFaceRangeToRHC( ( size_t ) pMod->faceOffset, ( size_t )( pMod->faceOffset + pMod->numFaces ) );
    }
}

void Quake3Map::convertFaceRangeToRHC( size_t start, size_t end )
{
    for ( size_t i = start; i < end; ++i )
    {
        BspFace* face = mFaceBuffer + i;

        q3cToRightHand( face->lightmapOrigin );
        q3cToRightHand( face->normal );

        // Corresponding Vertices
        size_t endVertices = face->vertexOffset + face->numVertices;

        for ( size_t v = 0; v < endVertices; ++v )
        {
            BspVertex* vertex = mVertexBuffer + v;

            q3cToRightHand( vertex->normal );
            q3cToRightHand( vertex->position );
        }

        // Corresponding Mesh Vertices ( TODO )
    }
}

