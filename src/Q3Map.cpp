#include <Q3Map.h>
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
    mModelBuffer = ( BspVertex* )( mDataBuffer + mHeader->lumpEntries[ BSP_LUMP_MODELS ].offset );
    mFaceBuffer = ( BspFace* )( mDataBuffer + mHeader->lumpEntries[ BSP_LUMP_FACES ].offset );

    mNumNodes = mHeader->lumpEntries[ BSP_LUMP_NODE ].numBytes / sizeof( BspNode );
    mNumLeaves = mHeader->lumpEntries[ BSP_LUMP_NODE ].numBytes / sizeof( BspLeaf );
    mNumPlanes = mHeader->lumpEntries[ BSP_LUMP_PLANE ].numBytes / sizeof( BspPlane );
    mNumVertices = mHeader->lumpEntries[ BSP_LUMP_VERTEXES ].numBytes / sizeof( BspVertex );
    mNumModels = mHeader->lumpEntries[ BSP_LUMP_MODELS ].numBytes / sizeof( BspModel );
    mNumFaces = mHeader->lumpEntries[ BSP_LUMP_FACES ].numBytes / sizeof( BspFace );

}
