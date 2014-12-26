#include "q3bsp.h"
#include "log.h"
#include "glutil.h"
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

static void SwizzleCoords( vec3f_t& v )
{
    float tmp = v.y;

    v.y = v.z;
    v.z = -tmp;
}

// Straight outta copypasta ( for integer vectors )
static void SwizzleCoords( vec3i_t& v )
{
    int tmp = v.y;

    v.y = v.z;
    v.z = -tmp;
}

static void ScaleCoords( vec3f_t& v, float scale )
{
    v.x *= scale;
    v.y *= scale;
    v.z *= scale;
}

static void ScaleCoords( vec2f_t& v, float scale )
{
    v.x *= scale;
    v.y *= scale;
}

static void ScaleCoords( vec3i_t& v, int scale )
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
	   visdata( NULL ),
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

		delete[] visdata->bitsets;
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

		GL_CHECK( glDeleteTextures( glTextures.size(), &glTextures[ 0 ] ) );

		delete[] mapBuffer;
		mapBuffer = nullptr;

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

	// Open file, verify it if we succeed
    FILE* file = fopen( filepath.c_str(), "rb" );

    if ( !file )
    {
        ERROR("Failed to open %s\n", filepath.c_str() );
    }

	fseek( file, 0, SEEK_END );
	size_t fsize = ftell( file );
	mapBuffer = new byte[ fsize ]();
	fseek( file, 0, SEEK_SET );
	fread( mapBuffer, fsize, 1, file );
	rewind( file );
	//fclose( file );

	memcpy( &header, mapBuffer, sizeof( bspHeader_t ) );

    if ( header.id[ 0 ] != 'I' || header.id[ 1 ] != 'B' || header.id[ 2 ] != 'S' || header.id[ 3 ] != 'P' )
    {
        ERROR( "Header ID does NOT match \'IBSP\'. ID read is: %s \n", header.id );
    }

    if ( header.version != BSP_Q3_VERSION )
    {
        ERROR( "Header version does NOT match %i. Version found is %i\n", BSP_Q3_VERSION, header.version );
    }

	// Read map data, swizzle coordinates from Z UP axis to Y UP axis in a right-handed system.
	// Scale anything as necessary (or desired)
    entities.infoString = ( char* )( mapBuffer + header.directories[ BSP_LUMP_ENTITIES ].offset );
    entityStringLen = header.directories[ BSP_LUMP_ENTITIES ].length / sizeof( char );

    textures = ( bspTexture_t* )( mapBuffer + header.directories[ BSP_LUMP_TEXTURES ].offset ); 
	numTextures = header.directories[ BSP_LUMP_TEXTURES ].length / sizeof( bspTexture_t );

    nodes = ( bspNode_t* )( mapBuffer + header.directories[ BSP_LUMP_NODES ].offset );
	numNodes = header.directories[ BSP_LUMP_NODES ].length / sizeof( bspNode_t );

    for ( int i = 0; i < numNodes; ++i )
    {
        ScaleCoords( nodes[ i ].boxMax, scale );
        ScaleCoords( nodes[ i ].boxMin, scale );

        SwizzleCoords( nodes[ i ].boxMax );
        SwizzleCoords( nodes[ i ].boxMin );
    }
	
    leaves = ( bspLeaf_t* )( mapBuffer + header.directories[ BSP_LUMP_LEAVES ].offset );
	numLeaves = header.directories[ BSP_LUMP_LEAVES ].length / sizeof( bspLeaf_t );

    for ( int i = 0; i < numLeaves; ++i )
    {
        ScaleCoords( leaves[ i ].boxMax, scale );
        ScaleCoords( leaves[ i ].boxMin, scale );

        SwizzleCoords( leaves[ i ].boxMax );
        SwizzleCoords( leaves[ i ].boxMin );
    }

	planes = ( bspPlane_t* )( mapBuffer + header.directories[ BSP_LUMP_PLANES ].offset );
    numPlanes = header.directories[ BSP_LUMP_PLANES ].length / sizeof( bspPlane_t );

    for ( int i = 0; i < numPlanes; ++i )
    {
        planes[ i ].distance *= scale;
        ScaleCoords( planes[ i ].normal, ( float ) scale );
        SwizzleCoords( planes[ i ].normal );
    }
	
    vertexes = ( bspVertex_t* )( mapBuffer + header.directories[ BSP_LUMP_VERTEXES ].offset );
	numVertexes = header.directories[ BSP_LUMP_VERTEXES ].length / sizeof( bspVertex_t );
	
	for ( int i = 0; i < numVertexes; ++i )
	{
		ScaleCoords( vertexes[ i ].texCoords[ 0 ], ( float ) scale );
        ScaleCoords( vertexes[ i ].texCoords[ 1 ], ( float ) scale );
        ScaleCoords( vertexes[ i ].normal, ( float ) scale );
        ScaleCoords( vertexes[ i ].position, ( float ) scale );

        SwizzleCoords( vertexes[ i ].position );
        SwizzleCoords( vertexes[ i ].normal );
	}
	
    models = ( bspModel_t* )( mapBuffer + header.directories[ BSP_LUMP_MODELS ].offset );
	numModels = header.directories[ BSP_LUMP_MODELS ].length / sizeof( bspModel_t );

    for ( int i = 0; i < numModels; ++i )
    {
        ScaleCoords( models[ i ].boxMax, ( float ) scale );
        ScaleCoords( models[ i ].boxMin, ( float ) scale );

        SwizzleCoords( models[ i ].boxMax );
        SwizzleCoords( models[ i ].boxMin );
    }
    
    faces = ( bspFace_t* )( mapBuffer + header.directories[ BSP_LUMP_FACES ].offset );
	numFaces = header.directories[ BSP_LUMP_FACES ].length / sizeof( bspFace_t );

    for ( int i = 0; i < numFaces; ++i )
    {
        ScaleCoords( faces[ i ].normal, ( float ) scale );
        ScaleCoords( faces[ i ].lightmapOrigin, ( float ) scale );
        ScaleCoords( faces[ i ].lightmapStVecs[ 0 ], ( float ) scale );
        ScaleCoords( faces[ i ].lightmapStVecs[ 1 ], ( float ) scale );

        SwizzleCoords( faces[ i ].normal );
        SwizzleCoords( faces[ i ].lightmapOrigin );
        SwizzleCoords( faces[ i ].lightmapStVecs[ 0 ] );
        SwizzleCoords( faces[ i ].lightmapStVecs[ 1 ] );
    }

    leafFaces = ( bspLeafFace_t* )( mapBuffer + header.directories[ BSP_LUMP_LEAF_FACES ].offset );
	numLeafFaces = header.directories[ BSP_LUMP_LEAF_FACES ].length / sizeof( bspLeafFace_t );

    meshVertexes = ( bspMeshVertex_t* )( mapBuffer + header.directories[ BSP_LUMP_MESH_VERTEXES ].offset );
	numMeshVertexes = header.directories[ BSP_LUMP_MESH_VERTEXES ].length / sizeof( bspMeshVertex_t );

	effectShaders = ( bspEffect_t* )( mapBuffer + header.directories[ BSP_LUMP_EFFECTS ].offset );
	numEffectShaders = header.directories[ BSP_LUMP_EFFECTS ].length / sizeof( bspEffect_t );

    visdata = ( bspVisdata_t* )( mapBuffer + header.directories[ BSP_LUMP_VISDATA ].offset );
	numVisdataVecs = header.directories[ BSP_LUMP_VISDATA ].length;
    
	// Reading the last portion of the data from the file directly has appeared to produce better results.
	// Not quite sure why, admittedly. See: http://stackoverflow.com/questions/27653440/mapping-data-to-an-offset-of-a-byte-buffer-allocated-for-an-entire-file-versus-r 
	// for the full story
	fseek( file, header.directories[ BSP_LUMP_VISDATA ].offset + sizeof( int ) * 2, SEEK_SET );
    
	int size = visdata->numVectors * visdata->sizeVector;
    visdata->bitsets = new byte[ size ]();
	fread( visdata->bitsets, size, 1, file );

	fclose( file );

    mapAllocated = true;

	glFaces.resize( numFaces );

	// Generate vbos; we simply cache the data already used for any polygon or mesh faces. For faces
	// which aren't of these two categories, we leave them be.
	for ( int i = 0; i < numFaces; ++i )
	{
		bspFace_t* face = faces + i;

		if ( face->type == BSP_FACE_TYPE_POLYGON || face->type == BSP_FACE_TYPE_MESH )
		{
			glFaces[ i ].indices.resize( face->numMeshVertexes );
			for ( int j = 0; j < face->numMeshVertexes; ++j )
				glFaces[ i ].indices[ j ] = face->vertexOffset + meshVertexes[ face->meshVertexOffset + j ].offset;
		}
	}
}

/*
=====================================================

Q3BspParser::GenTextures

Generates OpenGL texture data for map faces

=====================================================
*/

void Q3BspMap::GenTextures( const string &mapFilePath )
{
	//GL_CHECK( glPixelStorei( GL_UNPACK_ALIGNMENT, 1 ) );

	// Some might consider this a hack; however, it works. It's also simpler, portable, and more performant than traversing a number of directories
	// using an OS-specific API
	static const char* validImgExt[] = 
	{
		".jpg", ".png", ".tga", ".tiff", ".bmp"
	};

	const std::string& root = mapFilePath.substr( 0, mapFilePath.find_last_of( '/' ) ) + "/../";

	glTextures.resize( numTextures, 0 );
	GL_CHECK( glGenTextures( glTextures.size(), &glTextures[ 0 ] ) );

	for ( int t = 0; t < numTextures; t++ )
	{
		FILE* tf;

		std::string fname( textures[ t ].filename );

		const std::string& texPath = root + fname;
		
		// If we don't have a file extension appended in the name,
		// try to find one for it which is valid
		if ( fname.find_last_of( '.' ) == std::string::npos )
		{
			for ( int i = 0; i < SIGNED_LEN( validImgExt ); ++i )
			{
				const std::string& str = texPath + std::string( validImgExt[ i ] );

				tf = fopen( str.c_str(), "rb" );
			
				// If we found a match, bail
				if ( tf )
					break;
			}
		}
		else
		{
			tf = fopen( texPath.c_str(), "rb" );
		}
		
		// Stub out the texture for this iteration by continue; warn user
		if ( !tf )
			goto FAIL_WARN;

		// Load image
		int width, height, bpp;
		byte* imagePixels = stbi_load_from_file( tf, &width, &height, &bpp, STBI_default );

		fclose( tf );

		if ( !imagePixels )
			 goto FAIL_WARN;

		GLenum fmt;
		GLenum internalFmt;

		switch ( bpp )
		{
		case 1:
			fmt = GL_R;
			internalFmt = GL_R8; 
			break;
		case 3:
			internalFmt = GL_RGB8;
			fmt = GL_RGB;
			break;
		case 4:
			internalFmt = GL_RGBA8;
			fmt = GL_RGBA;
			break;
		default:
			ERROR( "Unsupported bits per pixel of %i specified; this needs to be fixed. For image file \'%s\'", bpp, texPath.c_str() );
			break;
		}
		
		GL_CHECK( glBindTexture( GL_TEXTURE_2D, glTextures[ t ] ) );

		GL_CHECK( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR ) );
		GL_CHECK( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR ) );
		GL_CHECK( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT ) );
		GL_CHECK( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT ) );

		GL_CHECK( glTexImage2D( GL_TEXTURE_2D, 0, internalFmt, width, height, 0, fmt, GL_UNSIGNED_BYTE, imagePixels ) );

		continue;

FAIL_WARN:
		WARNING( "Could not find a file extension for \'%s\'", texPath.c_str() );
		GL_CHECK( glDeleteTextures( 1, &glTextures[ t ] ) );
		glTextures[ t ] = 0;
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
        return true;

    int i = ( sourceCluster * visdata->sizeVector ) + ( testCluster >> 3 );

    byte visSet = visdata->bitsets[ i ];

    return ( visSet & ( 1 << ( testCluster & 7 ) ) ) != 0;
}

