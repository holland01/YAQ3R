#include "q3bsp.h"
#include "log.h"
#include "mtrand.h"
#include "extern/stb_image.c"

using namespace std;

/*
=====================================================

SwizzleCoords

Convert 3D vector formatted in Quake 3 coordinate system to a
right-handed coordinate system 3D vector.

original:       { x => -left/+right, y => -backward/+forward, z => -down/+up }
right-handed:   { x => -left/+right, y => -down/+up,          z => +backward/-forward }

=====================================================
*/

static void SwizzleCoords( float3_t& v )
{
    float tmp = v.y;

    v.y = v.z;
    v.z = -tmp;
}

// Straight outta copypasta ( for integer vectors )
static void SwizzleCoords( int3_t& v )
{
    int tmp = v.y;

    v.y = v.z;
    v.z = -tmp;
}

static void ScaleCoords( float3_t& v, float scale )
{
    v.x *= scale;
    v.y *= scale;
    v.z *= scale;
}

static void ScaleCoords( float2_t& v, float scale )
{
    v.x *= scale;
    v.y *= scale;
}

static void ScaleCoords( int3_t& v, float scale )
{
    v.x *= scale;
    v.y *= scale;
    v.z *= scale;
}

/*
=====================================================

Q3BspParser::Q3BspParser

=====================================================
*/

Q3BspMap::Q3BspMap( void )
     : nodes( NULL ),
       leaves( NULL ),
       planes( NULL ),
       vertexes( NULL ),
       textures( NULL ),
       models( NULL ),
       faces( NULL ),
       leafFaces( NULL ),
       meshVertexes( NULL ),
       apiTextures( NULL ),
       mapAllocated( false )
{
    entities.infoString = NULL;
}

/*
=====================================================

Q3BspParser::~Q3BspParser

=====================================================
*/

Q3BspMap::~Q3BspMap( void )
{
    DestroyMap();
}

/*
=====================================================

Q3BspParser::DestroyMap

Free all dynamically allocated data, and zero-out
other data, thus prepping the map for re-use if need be.

=====================================================
*/

void Q3BspMap::DestroyMap( void )
{
    if ( mapAllocated )
    {
        if ( apiTextures )
        {
            glDeleteTextures( numTextures, apiTextures );
            free( apiTextures );
        }

        free( entities.infoString );
        free( effectShaders );

        free( nodes );
        free( leaves );
        free( planes );

        free( vertexes );
        free( textures );
        free( models );
        free( faces );

        free( leafFaces );
        free( meshVertexes );

        free( visdata );

        apiTextures = NULL;

        entities.infoString = NULL;
        effectShaders = NULL;

        nodes = NULL;
        leaves = NULL;
        planes = NULL;

        vertexes = NULL;
        textures = NULL;
        models = NULL;
        faces = NULL;

        leafFaces = NULL;
        meshVertexes = NULL;

        visdata = NULL;

        numEffectShaders = 0;

        numNodes = 0;
        numLeaves = 0;
        numPlanes = 0;

        numVertexes = 0;
        numTextures = 0;
        numModels = 0;
        numFaces = 0;

        numLeafFaces = 0;
        numMeshVertexes = 0;

        numVisdataVecs = 0;

        mapAllocated = false;
    }
}

/*
=====================================================

Q3BspParser::Read

            read all map data into its respective
            buffer, defined by structs.

            "divisionScale" param is used to pre-divide
            vertex, normals, and bounding box data
            in the map by a specified size.

=====================================================
*/

void Q3BspMap::Read( const std::string& filepath, const int scale )
{   
    assert( scale != 0 );

    float fScale = scale;

    FILE* file = fopen( filepath.c_str(), "rb" );

    if ( !file )
    {
        ERROR("Failed to open %s\n", filepath.c_str() );
    }

    fread( &header, sizeof( bspHeader_t ), 1, file );

    if ( header.id[ 0 ] != 'I' || header.id[ 1 ] != 'B' || header.id[ 2 ] != 'S' || header.id[ 3 ] != 'P' )
    {
        ERROR( "Header ID does NOT match \'IBSP\'. ID read is: %s \n", header.id );
    }

    if ( header.version != BSP_Q3_VERSION )
    {
        ERROR( "Header version does NOT match %i. Version found is %i\n", BSP_Q3_VERSION, header.version );
    }

    entities.infoString = ( char* ) malloc( header.directories[ BSP_LUMP_ENTITIES ].length );
    entityStringLen = header.directories[ BSP_LUMP_ENTITIES ].length / sizeof( char );
    fseek( file, header.directories[ BSP_LUMP_ENTITIES ].offset, SEEK_SET );
    fread( entities.infoString, header.directories[ BSP_LUMP_ENTITIES ].length, 1, file );

    textures = ( bspTexture_t* )malloc( header.directories[ BSP_LUMP_TEXTURES ].length );
    numTextures = header.directories[ BSP_LUMP_TEXTURES ].length / sizeof( bspTexture_t );
    fseek( file, header.directories[ BSP_LUMP_TEXTURES ].offset, SEEK_SET );
    fread( textures, header.directories[ BSP_LUMP_TEXTURES ].length, 1, file );

    nodes = ( bspNode_t* )malloc( header.directories[ BSP_LUMP_NODES ].length );
    numNodes = header.directories[ BSP_LUMP_NODES ].length / sizeof( bspNode_t );
    fseek( file, header.directories[ BSP_LUMP_NODES ].offset, SEEK_SET );
    fread( nodes, header.directories[ BSP_LUMP_NODES ].length, 1, file );

    for ( int i = 0; i < numNodes; ++i )
    {

        ScaleCoords( nodes[ i ].boxMax, scale );
        ScaleCoords( nodes[ i ].boxMin, scale );

        SwizzleCoords( nodes[ i ].boxMax );
        SwizzleCoords( nodes[ i ].boxMin );
    }

    leaves = ( bspLeaf_t* )malloc( header.directories[ BSP_LUMP_LEAVES ].length );
    numLeaves = header.directories[ BSP_LUMP_LEAVES ].length / sizeof( bspLeaf_t );
    fseek( file, header.directories[ BSP_LUMP_LEAVES ].offset, SEEK_SET );
    fread( leaves, header.directories[ BSP_LUMP_LEAVES ].length, 1, file );

    for ( int i = 0; i < numLeaves; ++i )
    {
        ScaleCoords( leaves[ i ].boxMax, scale );
        ScaleCoords( leaves[ i ].boxMin, scale );

        SwizzleCoords( leaves[ i ].boxMax );
        SwizzleCoords( leaves[ i ].boxMin );
    }

    planes = ( bspPlane_t* )malloc( header.directories[ BSP_LUMP_PLANES ].length );
    numPlanes = header.directories[ BSP_LUMP_PLANES ].length / sizeof( bspPlane_t );
    fseek( file, header.directories[ BSP_LUMP_PLANES ].offset, SEEK_SET );
    fread( planes, header.directories[ BSP_LUMP_PLANES ].length, 1, file );

    for ( int i = 0; i < numPlanes; ++i )
    {
        planes[ i ].distance *= scale;
        ScaleCoords( planes[ i ].normal, fScale );
        SwizzleCoords( planes[ i ].normal );
    }

    vertexes = ( bspVertex_t* ) malloc( header.directories[ BSP_LUMP_VERTEXES ].length );
    numVertexes = header.directories[ BSP_LUMP_VERTEXES ].length / sizeof( bspVertex_t );
    fseek( file, header.directories[ BSP_LUMP_VERTEXES ].offset, SEEK_SET );
    fread( vertexes, header.directories[ BSP_LUMP_VERTEXES ].length, 1, file );

    for ( int i = 0; i < numVertexes; ++i )
    {
        ScaleCoords( vertexes[ i ].texCoord, scale );
        ScaleCoords( vertexes[ i ].lightmapCoord, scale );
        ScaleCoords( vertexes[ i ].normal, scale );
        ScaleCoords( vertexes[ i ].position, scale );

        SwizzleCoords( vertexes[ i ].position );
        SwizzleCoords( vertexes[ i ].normal );
    }

    models = ( bspModel_t* )malloc( header.directories[ BSP_LUMP_MODELS ].length );
    numModels = header.directories[ BSP_LUMP_MODELS ].length / sizeof( bspModel_t );
    fseek( file, header.directories[ BSP_LUMP_MODELS ].offset, SEEK_SET );
    fread( models, header.directories[ BSP_LUMP_MODELS ].length, 1, file );

    for ( int i = 0; i < numModels; ++i )
    {
        ScaleCoords( models[ i ].boxMax, scale );
        ScaleCoords( models[ i ].boxMin, scale );

        SwizzleCoords( models[ i ].boxMax );
        SwizzleCoords( models[ i ].boxMin );
    }

    faces = ( bspFace_t* )malloc( header.directories[ BSP_LUMP_FACES ].length );
    numFaces = header.directories[ BSP_LUMP_FACES ].length / sizeof( bspFace_t );
    fseek( file, header.directories[ BSP_LUMP_FACES ].offset, SEEK_SET );
    fread( faces, header.directories[ BSP_LUMP_FACES ].length, 1, file );

    for ( int i = 0; i < numFaces; ++i )
    {
        ScaleCoords( faces[ i ].normal, scale );
        ScaleCoords( faces[ i ].lightmapOrigin, scale );
        ScaleCoords( faces[ i ].lightmapStVecs[ 0 ], scale );
        ScaleCoords( faces[ i ].lightmapStVecs[ 1 ], scale );

        SwizzleCoords( faces[ i ].normal );
        SwizzleCoords( faces[ i ].lightmapOrigin );
        SwizzleCoords( faces[ i ].lightmapStVecs[ 0 ] );
        SwizzleCoords( faces[ i ].lightmapStVecs[ 1 ] );
    }

    leafFaces = ( bspLeafFace_t* )malloc( header.directories[ BSP_LUMP_LEAF_FACES ].length );
    numLeafFaces = header.directories[ BSP_LUMP_LEAF_FACES ].length / sizeof( bspLeafFace_t );
    fseek( file, header.directories[ BSP_LUMP_LEAF_FACES ].offset, SEEK_SET );
    fread( leafFaces, header.directories[ BSP_LUMP_LEAF_FACES ].length, 1, file );

    meshVertexes = ( bspMeshVertex_t* )malloc( header.directories[ BSP_LUMP_MESH_VERTEXES ].length );
    numMeshVertexes = header.directories[ BSP_LUMP_MESH_VERTEXES ].length / sizeof( bspMeshVertex_t );
    fseek( file, header.directories[ BSP_LUMP_MESH_VERTEXES ].offset, SEEK_SET );
    fread( meshVertexes, header.directories[ BSP_LUMP_MESH_VERTEXES ].length, 1, file );

    effectShaders = ( bspEffect_t* )malloc( header.directories[ BSP_LUMP_EFFECTS ].length );
    numEffectShaders = header.directories[ BSP_LUMP_EFFECTS ].length / sizeof( bspEffect_t );
    fseek( file, header.directories[ BSP_LUMP_EFFECTS ].offset, SEEK_SET );
    fread( effectShaders, header.directories[ BSP_LUMP_EFFECTS ].length, 1, file );

    visdata = ( bspVisdata_t* )malloc( header.directories[ BSP_LUMP_VISDATA ].length );
    numVisdataVecs = header.directories[ BSP_LUMP_VISDATA ].length;
    fseek( file, header.directories[ BSP_LUMP_VISDATA ].offset, SEEK_SET );
    fread( visdata, sizeof( int ), 2, file );
    int size = visdata->numVectors * visdata->sizeVector;
    visdata->bitsets = ( byte* )malloc( size );
    fread( visdata->bitsets, size, 1, file );

    mapAllocated = true;

    fclose( file );

   //LogBSPData( BSP_LUMP_VERTEXES, ( void* ) vertexes, numVertexes );
   //LogBSPData( BSP_LUMP_MESH_VERTEXES, ( void* ) meshVertexes, numMeshVertexes );
   LogBSPData( BSP_LUMP_ENTITIES, ( void* ) entities.infoString, entityStringLen );
   LogBSPData( BSP_LUMP_EFFECTS, ( void* ) effectShaders, numEffectShaders );
   LogBSPData( BSP_LUMP_TEXTURES, ( void* ) textures, numTextures );

}

/*
=====================================================

Q3BspParser::GenTextures

Generates OpenGL texture data for map faces

=====================================================
*/

void Q3BspMap::GenTextures( const string &mapFilePath )
{
    // extract (relative) root directory of map file in filepath string;
    // we append 1 at the end because we use the index as a
    // buffer length
    int dirRootLen = mapFilePath.find_last_of( '/' ) + 1;
    string relMapDirPath = mapFilePath.substr( 0, dirRootLen );
    relMapDirPath.append( "../" );

    apiTextures = ( GLuint* ) malloc( sizeof( GLuint ) * numTextures );
    glGenTextures( numTextures, apiTextures );

    string cwdpath;

    {
        char buf[ CHAR_MAX ];
        memset( buf, 0, CHAR_MAX );
        getcwd( buf, CHAR_MAX );

        cwdpath.append( buf );
        cwdpath.append( "/" );
    }

    // Iterate through the directory location of each texture so we can uncover its
    // extension and load it accordingly.
    for ( int i = 0; i < numTextures; ++i )
    {
        string texPath = relMapDirPath;
        texPath.append( textures[ i ].filename );

        bool finished = false;
        {
            // Comparison between filename in the texPath (sans root dir) and d_name param of "entry" var
            const string& cmp = texPath.substr( texPath.find_last_of( '/' ) + 1 );
            const string& dir = cwdpath + texPath.substr( 0, texPath.find_last_of( '/' ) );

            DIR* d;

            struct dirent* entry;

            d = opendir( dir.c_str() );

            // We have an invalid directory if this is the case
            if ( !d )
                continue;

            while ( ( entry = readdir( d ) ) != NULL )
            {
                string fname( entry->d_name );

                int extIndex = fname.find_last_of( '.' );

                if ( extIndex != -1 )
                {
                    const string& ext = fname.substr( extIndex );
                    const string& file = fname.substr( 0, extIndex );

                    finished = file == cmp;

                    if ( finished )
                    {
                        texPath.append( ext );
                        break;
                    }
                }
            }
        }

        if ( !finished )
            continue;

        int width, height, comp;

        unsigned char* pixels = stbi_load( texPath.c_str(), &width, &height, &comp, 0 );

        GLenum fmt, ifmt;

        switch ( comp )
        {
            case 3:
                ifmt = GL_RGB8;
                fmt = GL_RGB;
                break;
            default:
                ifmt = GL_RGBA8;
                fmt = GL_RGBA;
                break;
        }

        glBindTexture( GL_TEXTURE_2D, apiTextures[ i ] );
        glTexStorage2D( GL_TEXTURE_2D, 1, ifmt, width, height );
        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, width, height, fmt, GL_UNSIGNED_BYTE, pixels );

        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

        glBindTexture( GL_TEXTURE_2D, 0 );
    }
}

/*
=====================================================

Q3BspParser::SetVertexColorIf

Set the color of all vertices with a given alpha channel value
by a given RGB color, using a predicate function pointer.

Color and channel specified are within the range [0, 255]

=====================================================
*/

void Q3BspMap::SetVertexColorIf( bool ( predicate )( unsigned char* ), const glm::u8vec3& rgbColor )
{
    for ( int i = 0; i < numVertexes; ++i )
    {
        if ( ( *predicate )( vertexes[ i ].color ) )
        {
            vertexes[ i ].color[ 0 ] = rgbColor.r;
            vertexes[ i ].color[ 1 ] = rgbColor.g;
            vertexes[ i ].color[ 2 ] = rgbColor.b;
        }
    }
}

/*
=====================================================

Q3BspParser::FindClosestLeaf

Find the closest map leaf-node to the given camera position.
The leaf-node returned is further evaluated to detect faces the
camera is looking at.

=====================================================
*/

bspLeaf_t* Q3BspMap::FindClosestLeaf( const glm::vec3& camPos )
{
    int nodeIndex = 0;

    while ( nodeIndex >= 0 )
    {
        const bspNode_t* const node = nodes + nodeIndex;
        const bspPlane_t* const plane = planes + node->plane;

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

Q3BspParser::IsClusterVisible

Check to see if param sourceCluster can "see" param testCluster.
The algorithm used is standard and can be derived from the documentation
found in the link posted in q3bsp.h.

=====================================================
*/

bool Q3BspMap::IsClusterVisible( int sourceCluster, int testCluster )
{
    if ( !visdata->bitsets || ( sourceCluster < 0 ) )
    {
        return true;
    }

    int i = ( sourceCluster * visdata->sizeVector ) + ( testCluster >> 3 );

    byte visSet = visdata->bitsets[ i ];

    return ( visSet & ( 1 << ( testCluster & 7 ) ) ) != 0;
}

