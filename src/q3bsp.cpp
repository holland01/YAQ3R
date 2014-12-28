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
     :	mapAllocated( false ),
		data( {} )
{
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
		delete[] data.visdata->bitsets;
        delete[] data.buffer;	
		memset( &data, 0, sizeof( mapData_t ) );

		GL_CHECK( glDeleteTextures( glTextures.size(), &glTextures[ 0 ] ) );
		GL_CHECK( glDeleteTextures( glLightmaps.size(), &glLightmaps[ 0 ] ) );

		glTextures.clear();
		glLightmaps.clear();
		glFaces.clear();
		
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

void Q3BspMap::Read( const std::string& filepath, const int scale, uint32_t loadFlags )
{   
	if ( IsAllocated() )
		DestroyMap();

	assert( scale != 0 );

	// Open file, verify it if we succeed
    FILE* file = fopen( filepath.c_str(), "rb" );

    if ( !file )
    {
        ERROR("Failed to open %s\n", filepath.c_str() );
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
        ERROR( "Header ID does NOT match \'IBSP\'. ID read is: %s \n", data.header->id );
    }

    if ( data.header->version != BSP_Q3_VERSION )
    {
        ERROR( "Header version does NOT match %i. Version found is %i\n", BSP_Q3_VERSION, data.header->version );
    }

	// Read map data, swizzle coordinates from Z UP axis to Y UP axis in a right-handed system.
	// Scale anything as necessary (or desired)
    data.entities.infoString = ( char* )( data.buffer + data.header->directories[ BSP_LUMP_ENTITIES ].offset );
    data.entityStringLen = data.header->directories[ BSP_LUMP_ENTITIES ].length / sizeof( char );

    data.textures = ( bspTexture_t* )( data.buffer + data.header->directories[ BSP_LUMP_TEXTURES ].offset ); 
	data.numTextures = data.header->directories[ BSP_LUMP_TEXTURES ].length / sizeof( bspTexture_t );

    data.nodes = ( bspNode_t* )( data.buffer + data.header->directories[ BSP_LUMP_NODES ].offset );
	data.numNodes = data.header->directories[ BSP_LUMP_NODES ].length / sizeof( bspNode_t );

    for ( int i = 0; i < data.numNodes; ++i )
    {
        ScaleCoords( data.nodes[ i ].boxMax, scale );
        ScaleCoords( data.nodes[ i ].boxMin, scale );

        SwizzleCoords( data.nodes[ i ].boxMax );
        SwizzleCoords(data. nodes[ i ].boxMin );
    }
	
    data.leaves = ( bspLeaf_t* )( data.buffer + data.header->directories[ BSP_LUMP_LEAVES ].offset );
	data.numLeaves = data.header->directories[ BSP_LUMP_LEAVES ].length / sizeof( bspLeaf_t );

    for ( int i = 0; i < data.numLeaves; ++i )
    {
        ScaleCoords( data.leaves[ i ].boxMax, scale );
        ScaleCoords( data.leaves[ i ].boxMin, scale );

        SwizzleCoords( data.leaves[ i ].boxMax );
        SwizzleCoords( data.leaves[ i ].boxMin );
    }

	data.planes = ( bspPlane_t* )( data.buffer + data.header->directories[ BSP_LUMP_PLANES ].offset );
    data.numPlanes = data.header->directories[ BSP_LUMP_PLANES ].length / sizeof( bspPlane_t );

    for ( int i = 0; i < data.numPlanes; ++i )
    {
		data.planes[ i ].distance *= scale;
        ScaleCoords( data.planes[ i ].normal, ( float ) scale );
        SwizzleCoords( data.planes[ i ].normal );
    }
	
    data.vertexes = ( bspVertex_t* )( data.buffer + data.header->directories[ BSP_LUMP_VERTEXES ].offset );
	data.numVertexes = data.header->directories[ BSP_LUMP_VERTEXES ].length / sizeof( bspVertex_t );
	
	for ( int i = 0; i < data.numVertexes; ++i )
	{
		ScaleCoords( data.vertexes[ i ].texCoords[ 0 ], ( float ) scale );
        ScaleCoords( data.vertexes[ i ].texCoords[ 1 ], ( float ) scale );
        ScaleCoords( data.vertexes[ i ].normal, ( float ) scale );
        ScaleCoords( data.vertexes[ i ].position, ( float ) scale );

        SwizzleCoords( data.vertexes[ i ].position );
        SwizzleCoords( data.vertexes[ i ].normal );
	}
	
    data.models = ( bspModel_t* )( data.buffer + data.header->directories[ BSP_LUMP_MODELS ].offset );
	data.numModels = data.header->directories[ BSP_LUMP_MODELS ].length / sizeof( bspModel_t );

    for ( int i = 0; i < data.numModels; ++i )
    {
        ScaleCoords( data.models[ i ].boxMax, ( float ) scale );
        ScaleCoords( data.models[ i ].boxMin, ( float ) scale );

        SwizzleCoords( data.models[ i ].boxMax );
        SwizzleCoords( data.models[ i ].boxMin );
    }
    
    data.faces = ( bspFace_t* )( data.buffer + data.header->directories[ BSP_LUMP_FACES ].offset );
	data.numFaces = data.header->directories[ BSP_LUMP_FACES ].length / sizeof( bspFace_t );

    for ( int i = 0; i < data.numFaces; ++i )
    {
        ScaleCoords( data.faces[ i ].normal, ( float ) scale );
        ScaleCoords( data.faces[ i ].lightmapOrigin, ( float ) scale );
        ScaleCoords( data.faces[ i ].lightmapStVecs[ 0 ], ( float ) scale );
        ScaleCoords( data.faces[ i ].lightmapStVecs[ 1 ], ( float ) scale );

        SwizzleCoords( data.faces[ i ].normal );
        SwizzleCoords( data.faces[ i ].lightmapOrigin );
        SwizzleCoords( data.faces[ i ].lightmapStVecs[ 0 ] );
        SwizzleCoords( data.faces[ i ].lightmapStVecs[ 1 ] );
    }

    data.leafFaces = ( bspLeafFace_t* )( data.buffer + data.header->directories[ BSP_LUMP_LEAF_FACES ].offset );
	data.numLeafFaces = data.header->directories[ BSP_LUMP_LEAF_FACES ].length / sizeof( bspLeafFace_t );

    data.meshVertexes = ( bspMeshVertex_t* )( data.buffer + data.header->directories[ BSP_LUMP_MESH_VERTEXES ].offset );
	data.numMeshVertexes = data.header->directories[ BSP_LUMP_MESH_VERTEXES ].length / sizeof( bspMeshVertex_t );

	data.effectShaders = ( bspEffect_t* )( data.buffer + data.header->directories[ BSP_LUMP_EFFECTS ].offset );
	data.numEffectShaders = data.header->directories[ BSP_LUMP_EFFECTS ].length / sizeof( bspEffect_t );

	data.lightmaps = ( bspLightmap_t* )( data.buffer + data.header->directories[ BSP_LUMP_LIGHTMAPS ].offset );
	data.numLightmaps = data.header->directories[ BSP_LUMP_LIGHTMAPS ].length / sizeof( bspLightmap_t );

	data.lightvols = ( bspLightvol_t* )( data.buffer + data.header->directories[ BSP_LUMP_LIGHTVOLS ].offset );
	data.numLightvols = data.header->directories[ BSP_LUMP_LIGHTVOLS ].length / sizeof( bspLightvol_t );

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

    mapAllocated = true;

	glFaces.resize( data.numFaces );

	// Generate vbos; we simply cache the data already used for any polygon or mesh faces. For faces
	// which aren't of these two categories, we leave them be.
	for ( int i = 0; i < data.numFaces; ++i )
	{
		bspFace_t* face = data.faces + i;

		if ( face->type == BSP_FACE_TYPE_POLYGON || face->type == BSP_FACE_TYPE_MESH )
		{
			glFaces[ i ].indices.resize( face->numMeshVertexes );
			for ( int j = 0; j < face->numMeshVertexes; ++j )
				glFaces[ i ].indices[ j ] = face->vertexOffset + data.meshVertexes[ face->meshVertexOffset + j ].offset;
		}
	}
	
	// Now, find and generate the textures. We first start with the image files.
	glTextures.resize( data.numTextures, 0 );
	GL_CHECK( glGenTextures( glTextures.size(), &glTextures[ 0 ] ) );

	// GL_CHECK( glPixelStorei( GL_UNPACK_ALIGNMENT, 1 ) );

	// Some might consider this a hack, and to be fair it kind of is. However, it works. It's also simpler, portable, and more performant than traversing a number of directories
	// using an OS-specific API. It's also only temporary :)
	{
		GLint oldAlign;
		GL_CHECK( glGetIntegerv( GL_UNPACK_ALIGNMENT, &oldAlign ) );
		GL_CHECK( glPixelStorei( GL_UNPACK_ALIGNMENT, 1 ) );

		static const char* validImgExt[] = 
		{
			".jpg", ".png", ".tga", ".tiff", ".bmp"
		};

		const std::string& root = filepath.substr( 0, filepath.find_last_of( '/' ) ) + "/../";

		for ( int t = 0; t < data.numTextures; t++ )
		{
			FILE* tf;

			std::string fname( data.textures[ t ].filename );

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

			GLenum rgb, rgba;
			
			if ( loadFlags & Q3LOAD_TEXTURE_SRGB )
			{
				rgb = GL_SRGB8;
				rgba = GL_SRGB8_ALPHA8;
			}
			else
			{
				rgb = GL_RGB8;
				rgba = GL_RGBA8;
			}

			switch ( bpp )
			{
			case 1:
				fmt = GL_R;
				internalFmt = GL_R8; 
				break;
			case 3:
				internalFmt = rgb;
				fmt = GL_RGB;
				break;
			case 4:
				internalFmt = rgba;
				fmt = GL_RGBA;
				break;
			default:
				ERROR( "Unsupported bits per pixel of %i specified; this needs to be fixed. For image file \'%s\'", bpp, texPath.c_str() );
				break;
			}
		
			GL_CHECK( glBindTexture( GL_TEXTURE_2D, glTextures[ t ] ) );

			int maxLevels = glm::min( ( int ) glm::log2( ( float ) width ), ( int ) glm::log2( ( float ) height ) ); 

			GLenum minFilter;
			if ( loadFlags & Q3LOAD_TEXTURE_MIPMAP )
			{
				int w = width;
				int h = height;

				for ( int mip = 0; h != 1 && w != 1; ++mip )
				{
					GL_CHECK( glTexImage2D( GL_TEXTURE_2D, mip, internalFmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, imagePixels ) );

					if ( h > 1 )
						h /= 2;

					if ( w > 1 )
						w /= 2;
				}

				GL_CHECK( glGenerateMipmap( GL_TEXTURE_2D ) );
				minFilter = GL_LINEAR_MIPMAP_LINEAR;
			}
			else
			{
				GL_CHECK( glTexImage2D( GL_TEXTURE_2D, 0, internalFmt, width, height, 0, fmt, GL_UNSIGNED_BYTE, imagePixels ) );
				minFilter = GL_LINEAR;
			}
		

			if ( loadFlags & Q3LOAD_TEXTURE_ANISOTROPY )
			{
				GLfloat maxSamples;
				GL_CHECK( glGetFloatv( GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxSamples ) );
				GL_CHECK( glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxSamples ) );
			}

			GL_CHECK( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter ) );
			GL_CHECK( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR ) );
			GL_CHECK( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT ) );
			GL_CHECK( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT ) );

			continue;

	FAIL_WARN:
			WARNING( "Could not find a file extension for \'%s\'", texPath.c_str() );
			GL_CHECK( glDeleteTextures( 1, &glTextures[ t ] ) );
			glTextures[ t ] = 0;
		}

		// Reset the alignment to maintain consistency
		GL_CHECK( glPixelStorei( GL_UNPACK_ALIGNMENT, oldAlign ) );
	}

	// And then generate all of the lightmaps
	glLightmaps.resize( data.numLightmaps, 0 );
	GL_CHECK( glGenTextures( glLightmaps.size(), &glLightmaps[ 0 ] ) );
	
	for ( int l = 0; l < data.numLightmaps; ++l )
	{	
		GL_CHECK( glBindTexture( GL_TEXTURE_2D, glLightmaps[ l ] ) );

		GL_CHECK( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR ) );
		GL_CHECK( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR ) );
		GL_CHECK( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT ) );
		GL_CHECK( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT ) );

		GL_CHECK( glTexImage2D( 
			GL_TEXTURE_2D, 
			0, 
			GL_RGB8, 
			BSP_LIGHTMAP_WIDTH, 
			BSP_LIGHTMAP_HEIGHT, 
			0, 
			GL_RGB, 
			GL_UNSIGNED_BYTE, 
			data.lightmaps[ l ].map ) );	
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
        const bspNode_t* const node = data.nodes + nodeIndex;
        const bspPlane_t* const plane = data.planes + node->plane;

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

    return &data.leaves[ nodeIndex ];
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
    if ( !data.visdata->bitsets || ( sourceCluster < 0 ) )
        return true;

    int i = ( sourceCluster * data.visdata->sizeVector ) + ( testCluster >> 3 );

    byte visSet = data.visdata->bitsets[ i ];

    return ( visSet & ( 1 << ( testCluster & 7 ) ) ) != 0;
}

