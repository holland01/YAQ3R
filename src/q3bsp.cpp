#include "q3bsp.h"
#include "aabb.h"
#include "io.h"
#include "effect_shader.h"

using namespace std;

static void SwizzleCoords( glm::vec3& v )
{
    float tmp = v.y;
    v.y = v.z;
    v.z = -tmp;
}

// Straight outta copypasta ( for integer vectors )
static void SwizzleCoords( glm::ivec3& v )
{
    int tmp = v.y;
    v.y = v.z;
    v.z = -tmp;
}

static void ScaleCoords( glm::vec3& v, float scale )
{
    v.x *= scale;
    v.y *= scale;
    v.z *= scale;
}

static void ScaleCoords( glm::vec2& v, float scale )
{
    v.x *= scale;
    v.y *= scale;
}

static void ScaleCoords( glm::ivec3& v, int scale )
{
    v.x *= scale;
    v.y *= scale;
    v.z *= scale;
}

//-------------------------------------------------------------------------------

Q3BspMap::Q3BspMap( void )
     :	mapAllocated( false ),
		data( {} )
{
}

Q3BspMap::~Q3BspMap( void )
{
    DestroyMap();
}

const shaderInfo_t* Q3BspMap::GetShaderInfo( int faceIndex ) const
{
	const bspFace_t* face = data.faces + faceIndex;

	if ( face->effect != -1 )
	{
		auto it = effectShaders.find( data.effectShaders[ face->effect ].name );

		if ( it != effectShaders.end() )
		{
			return &it->second;
		}
	}

	if ( face->texture != -1 )
	{
		auto it = effectShaders.find( data.textures[ face->texture ].name );
	
		if ( it != effectShaders.end() )
		{
			return &it->second;
		}
	}

	return nullptr;
}

void Q3BspMap::DestroyMap( void )
{
    if ( mapAllocated )
    {
		delete[] data.visdata->bitsets;
        delete[] data.buffer;	
        //memset( &data, 0, sizeof( mapData_t ) );

		mapAllocated = false;
    }
}

void Q3BspMap::Read( const std::string& filepath, const int scale )
{   
	if ( IsAllocated() )
		DestroyMap();

	data.basePath = filepath.substr( 0, filepath.find_last_of( '/' ) ) + "/../";
	ReadFile( filepath, scale );
	mapAllocated = true;
}

void Q3BspMap::WriteLumpToFile( uint32_t lump )
{
	FILE* f = nullptr;
	byte* mem = nullptr;
	std::string filepath;
	size_t numBytes = 0;
	(void)numBytes;

	switch ( lump )
	{
		case BSP_LUMP_ENTITIES:
			filepath = "log/entities.log";
			mem = ( byte* ) data.entities.infoString;
			numBytes = data.header->directories[ BSP_LUMP_ENTITIES ].length;
			break;

		default:
			return;
	}

	f = fopen( filepath.c_str(), "wb" );
	fprintf( f, "%s\n", mem );
}
  
void Q3BspMap::ReadFile( const std::string& filepath, const int scale )
{
	// Open file, verify it if we succeed
    FILE* file = fopen( filepath.c_str(), "rb" );

    if ( !file )
    {
        MLOG_ERROR( "Failed to open %s\n", filepath.c_str() );
    }

	fseek( file, 0, SEEK_END );
	size_t fsize = ftell( file );
	data.buffer = new byte[ fsize ]();
	fseek( file, 0, SEEK_SET );
	fread( data.buffer, fsize, 1, file );
	rewind( file );

	data.header = ( bspHeader_t* )data.buffer;

    if ( data.header->id[ 0 ] != 'I' || data.header->id[ 1 ] != 'B' || data.header->id[ 2 ] != 'S' || data.header->id[ 3 ] != 'P' )
    {
        MLOG_ERROR( "Header ID does NOT match \'IBSP\'. ID read is: %s \n", data.header->id );
    }

    if ( data.header->version != BSP_Q3_VERSION )
    {
        MLOG_ERROR( "Header version does NOT match %i. Version found is %i\n", BSP_Q3_VERSION, data.header->version );
    }

    //
    // Read map data
    //
    data.entities.infoString = ( char* )( data.buffer + data.header->directories[ BSP_LUMP_ENTITIES ].offset );
    data.entityStringLen = data.header->directories[ BSP_LUMP_ENTITIES ].length / sizeof( char );

    data.textures = ( bspTexture_t* )( data.buffer + data.header->directories[ BSP_LUMP_TEXTURES ].offset ); 
	data.numTextures = data.header->directories[ BSP_LUMP_TEXTURES ].length / sizeof( bspTexture_t );

    data.nodes = ( bspNode_t* )( data.buffer + data.header->directories[ BSP_LUMP_NODES ].offset );
	data.numNodes = data.header->directories[ BSP_LUMP_NODES ].length / sizeof( bspNode_t );
	
    data.leaves = ( bspLeaf_t* )( data.buffer + data.header->directories[ BSP_LUMP_LEAVES ].offset );
	data.numLeaves = data.header->directories[ BSP_LUMP_LEAVES ].length / sizeof( bspLeaf_t );

	data.planes = ( bspPlane_t* )( data.buffer + data.header->directories[ BSP_LUMP_PLANES ].offset );
    data.numPlanes = data.header->directories[ BSP_LUMP_PLANES ].length / sizeof( bspPlane_t );
	
    data.vertexes = ( bspVertex_t* )( data.buffer + data.header->directories[ BSP_LUMP_VERTEXES ].offset );
	data.numVertexes = data.header->directories[ BSP_LUMP_VERTEXES ].length / sizeof( bspVertex_t );
	
    data.models = ( bspModel_t* )( data.buffer + data.header->directories[ BSP_LUMP_MODELS ].offset );
	data.numModels = data.header->directories[ BSP_LUMP_MODELS ].length / sizeof( bspModel_t );

	data.numFaces = data.header->directories[ BSP_LUMP_FACES ].length / sizeof( bspFace_t );
    data.faces = ( bspFace_t* )( data.buffer + data.header->directories[ BSP_LUMP_FACES ].offset );

    data.leafFaces = ( bspLeafFace_t* )( data.buffer + data.header->directories[ BSP_LUMP_LEAF_FACES ].offset );
	data.numLeafFaces = data.header->directories[ BSP_LUMP_LEAF_FACES ].length / sizeof( bspLeafFace_t );

	data.leafBrushes = ( bspLeafBrush_t* )( data.buffer + data.header->directories[ BSP_LUMP_LEAF_BRUSHES ].offset );
	data.numLeafBrushes = data.header->directories[ BSP_LUMP_LEAF_BRUSHES ].length / sizeof( bspLeafBrush_t );

    data.meshVertexes = ( bspMeshVertex_t* )( data.buffer + data.header->directories[ BSP_LUMP_MESH_VERTEXES ].offset );
	data.numMeshVertexes = data.header->directories[ BSP_LUMP_MESH_VERTEXES ].length / sizeof( bspMeshVertex_t );

	data.effectShaders = ( bspEffect_t* )( data.buffer + data.header->directories[ BSP_LUMP_EFFECTS ].offset );
	data.numEffectShaders = data.header->directories[ BSP_LUMP_EFFECTS ].length / sizeof( bspEffect_t );

	data.lightmaps = ( bspLightmap_t* )( data.buffer + data.header->directories[ BSP_LUMP_LIGHTMAPS ].offset );
	data.numLightmaps = data.header->directories[ BSP_LUMP_LIGHTMAPS ].length / sizeof( bspLightmap_t );

	data.lightvols = ( bspLightvol_t* )( data.buffer + data.header->directories[ BSP_LUMP_LIGHTVOLS ].offset );
	data.numLightvols = data.header->directories[ BSP_LUMP_LIGHTVOLS ].length / sizeof( bspLightvol_t );

	data.brushes = ( bspBrush_t* )( data.buffer + data.header->directories[ BSP_LUMP_BRUSHES ].offset ); 
	data.numBrushes = data.header->directories[ BSP_LUMP_BRUSHES ].length / sizeof( bspBrush_t );

	data.brushSides = ( bspBrushSide_t* )( data.buffer + data.header->directories[ BSP_LUMP_BRUSH_SIDES ].offset );
	data.numBrushSides = data.header->directories[ BSP_LUMP_BRUSH_SIDES ].length / sizeof( bspBrushSide_t );

    data.visdata = ( bspVisdata_t* )( data.buffer + data.header->directories[ BSP_LUMP_VISDATA ].offset );
	data.numVisdataVecs = data.header->directories[ BSP_LUMP_VISDATA ].length;

	// Reading the last portion of the data from the file directly has appeared to produce better results.
	// Not quite sure why, admittedly. See: http://stackoverflow.com/questions/27653440/mapping-data-to-an-offset-of-a-byte-buffer-allocated-for-an-entire-file-versus-r 
	// for the full story
	fseek( file, data.header->directories[ BSP_LUMP_VISDATA ].offset + sizeof( int ) * 2, SEEK_SET );
    
	int size = data.visdata->numVectors * data.visdata->sizeVector;
    data.visdata->bitsets = new byte[ size ]();
	fread( data.visdata->bitsets, size, 1, file );

	fclose( file );

    //
    // swizzle coordinates from left-handed Z UP axis to right-handed Y UP axis. Also scale anything as necessary (or desired)
    //

    for ( int i = 0; i < data.numNodes; ++i )
    {
        ScaleCoords( data.nodes[ i ].boxMax, scale );
        ScaleCoords( data.nodes[ i ].boxMin, scale );

        SwizzleCoords( data.nodes[ i ].boxMax );
        SwizzleCoords( data.nodes[ i ].boxMin );
    }

    for ( int i = 0; i < data.numLeaves; ++i )
    {
        ScaleCoords( data.leaves[ i ].boxMax, scale );
        ScaleCoords( data.leaves[ i ].boxMin, scale );

        SwizzleCoords( data.leaves[ i ].boxMax );
        SwizzleCoords( data.leaves[ i ].boxMin );
    }

    for ( int i = 0; i < data.numPlanes; ++i )
    {
        data.planes[ i ].distance *= scale;
        ScaleCoords( data.planes[ i ].normal, ( float ) scale );
        SwizzleCoords( data.planes[ i ].normal );
    }

    for ( int i = 0; i < data.numVertexes; ++i )
    {
        ScaleCoords( data.vertexes[ i ].texCoords[ 0 ], ( float )scale );
        ScaleCoords( data.vertexes[ i ].texCoords[ 1 ], ( float )scale );
        ScaleCoords( data.vertexes[ i ].normal, ( float ) scale );
        ScaleCoords( data.vertexes[ i ].position, ( float ) scale );

        SwizzleCoords( data.vertexes[ i ].position );
        SwizzleCoords( data.vertexes[ i ].normal );
    }

    for ( int i = 0; i < data.numModels; ++i )
    {
        ScaleCoords( data.models[ i ].boxMax, ( float ) scale );
        ScaleCoords( data.models[ i ].boxMin, ( float ) scale );

        SwizzleCoords( data.models[ i ].boxMax );
        SwizzleCoords( data.models[ i ].boxMin );
    }

    for ( int i = 0; i < data.numFaces; ++i )
    {
        bspFace_t& face = data.faces[ i ];

        ScaleCoords( face.normal, ( float ) scale );
        ScaleCoords( face.lightmapOrigin, ( float ) scale );
        ScaleCoords( face.lightmapStVecs[ 0 ], ( float ) scale );
        ScaleCoords( face.lightmapStVecs[ 1 ], ( float ) scale );

        SwizzleCoords( face.normal );
        SwizzleCoords( face.lightmapOrigin );
        SwizzleCoords( face.lightmapStVecs[ 0 ] );
        SwizzleCoords( face.lightmapStVecs[ 1 ] );
    }

    LogBSPData( BSP_LUMP_EFFECTS, ( void* ) ( data.effectShaders ), data.numEffectShaders );
    LogBSPData( BSP_LUMP_ENTITIES, ( void *) ( data.entities.infoString ), -1 );
}

bspLeaf_t* Q3BspMap::FindClosestLeaf( const glm::vec3& camPos )
{
    int nodeIndex = 0;

    while ( nodeIndex >= 0 )
    {
        const bspNode_t* const node = &data.nodes[ nodeIndex ];
        const bspPlane_t* const plane = &data.planes[ node->plane ];

        // If the distance from the camera to the plane is >= 0,
        // then our needed camera data is in a leaf somewhere in front of this node,
        // otherwise it's behind the node somewhere.

        glm::vec3 planeNormal( plane->normal.x, plane->normal.y, plane->normal.z );

        float distance = glm::dot( planeNormal, camPos ) - plane->distance;

        if ( distance >= 0 )
		{
            nodeIndex = node->children[ 0 ];
		}
		else
		{
            nodeIndex = node->children[ 1 ];
		}
	}

    nodeIndex = -( nodeIndex + 1 );

    return &data.leaves[ nodeIndex ];
}

bool Q3BspMap::IsClusterVisible( int sourceCluster, int testCluster )
{
    if ( !data.visdata->bitsets || ( sourceCluster < 0 ) )
	{
        return true;
	}

    int i = ( sourceCluster * data.visdata->sizeVector ) + ( testCluster >> 3 );

    byte visSet = data.visdata->bitsets[ i ];

    return ( visSet & ( 1 << ( testCluster & 7 ) ) ) != 0;
}

