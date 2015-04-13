#include "q3bsp.h"
#include "log.h"
#include "glutil.h"

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

bspVertex_t::bspVertex_t( void )
	: bspVertex_t( glm::zero< glm::vec3 >(), glm::zero< glm::vec3 >(), glm::zero< glm::vec2 >(), glm::zero< glm::vec2 >(), glm::zero< glm::u8vec4 >() )
{
}

bspVertex_t::bspVertex_t( const glm::vec3& pos, const glm::vec3& norm, const glm::vec2& surfTexCoords, const glm::vec2& lightmapTexCoords, const glm::u8vec4& color_ )
	: position( pos ),
	  normal( norm ),
	  color( color_ )
{
	texCoords[ 0 ] = surfTexCoords;
	texCoords[ 1 ] = lightmapTexCoords;
}

bspVertex_t::bspVertex_t( const bspVertex_t& v )
	: position( v.position ),
	  normal( v.normal ),
	  color( v.color )
{
	memcpy( texCoords, v.texCoords, sizeof( texCoords ) );
}

bspVertex_t& bspVertex_t::operator=( bspVertex_t v )
{
	position = v.position;
	normal = v.normal;
	color = v.color;

	memcpy( texCoords, v.texCoords, sizeof( texCoords ) );

	return *this;
}

bspVertex_t operator +( const bspVertex_t& a, const bspVertex_t& b )
{
	bspVertex_t vert;
	
	vert.position = a.position + b.position;

	vert.color[ 0 ] = a.color[ 0 ];
	vert.color[ 1 ] = a.color[ 1 ];
	vert.color[ 2 ] = a.color[ 2 ];
	vert.color[ 3 ] = a.color[ 3 ];
	
	vert.normal = a.normal + b.normal;
	vert.texCoords[ 0 ] = a.texCoords[ 0 ] + b.texCoords[ 0 ];
    vert.texCoords[ 1 ] = a.texCoords[ 1 ] + b.texCoords[ 1 ];

	// TODO: lightmapCoords?

	return vert;
}

bspVertex_t operator -( const bspVertex_t& a, const bspVertex_t& b )
{
	bspVertex_t vert;
	
	vert.position = a.position - b.position;

	vert.color = a.color;
	
	vert.normal = a.normal - b.normal;
	vert.texCoords[ 0 ] = a.texCoords[ 0 ] - b.texCoords[ 0 ];
    vert.texCoords[ 1 ] = a.texCoords[ 1 ] - b.texCoords[ 1 ];

	return vert;
}

bspVertex_t operator *( const bspVertex_t& a, float b )
{
	bspVertex_t vert;
	
	vert.position = a.position * b;

	vert.normal = a.normal * b;
	vert.texCoords[ 0 ] = a.texCoords[ 0 ] * b;
    vert.texCoords[ 1 ] = a.texCoords[ 1 ] * b;

	vert.color = a.color;

	return vert;
}

bspVertex_t& operator += ( bspVertex_t& a, const bspVertex_t& b )
{
	a.position += b.position;
	a.normal += b.normal;
	a.texCoords[ 0 ] += b.texCoords[ 0 ];
	a.texCoords[ 1 ] += b.texCoords[ 1 ];

	return a;
}

bool operator == ( const bspVertex_t&a, const bspVertex_t& b )
{
	return a.position == b.position 
		&& a.texCoords[ 0 ] == b.texCoords[ 0 ]
		&& a.texCoords[ 1 ] == b.texCoords[ 1 ]
		&& a.normal == b.normal;
}

//-------------------------------------------------------------------------------

Q3BspMap::Q3BspMap( void )
     :	mapAllocated( false ),
		data( {} )
{
	glLightmapSampler = 0;
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
		memset( &data, 0, sizeof( mapData_t ) );

		GL_CHECK( glDeleteTextures( glTextures.size(), &glTextures[ 0 ] ) );
		GL_CHECK( glDeleteTextures( glLightmaps.size(), &glLightmaps[ 0 ] ) );
		GL_CHECK( glDeleteSamplers( glSamplers.size(), &glSamplers[ 0 ] ) );

		if ( glLightmapSampler > 0 )
		{	
			GL_CHECK( glDeleteSamplers( 1, &glLightmapSampler ) );
		}

		glTextures.clear();
		glLightmaps.clear();
		glFaces.clear();
		
		for ( auto dm: glDeformed )
		{
			if ( dm.second )
			{
				delete dm.second;
			}
		}

		glDeformed.clear();

        mapAllocated = false;
    }
}

void Q3BspMap::Read( const std::string& filepath, const int scale, uint32_t loadFlags )
{   
	if ( IsAllocated() )
		DestroyMap();

	data.basePath = filepath.substr( 0, filepath.find_last_of( '/' ) ) + "/../";
	ReadFile( filepath, scale );

	LoadShaders( &data, loadFlags, effectShaders );
	GenNonShaderTextures( loadFlags );
	GenRenderData();

	mapAllocated = true;
}

void Q3BspMap::GenRenderData( void )
{
	glFaces.resize( data.numFaces );

	// cache the data already used for any polygon or mesh faces, so we don't have to iterate through their index/vertex mapping every frame. For faces
	// which aren't of these two categories, we leave them be.
	for ( int i = 0; i < data.numFaces; ++i )
	{
		const bspFace_t* face = data.faces + i;

		if ( face->type == BSP_FACE_TYPE_MESH || face->type == BSP_FACE_TYPE_POLYGON )
		{
			glFaces[ i ].indices.resize( face->numMeshVertexes, 0 );
			glFaces[ i ].vertices.resize( face->numMeshVertexes );
			memset( &glFaces[ i ].controlPoints, 0, sizeof( bspVertex_t* ) * BSP_NUM_CONTROL_POINTS );

			for ( int j = 0; j < face->numMeshVertexes; ++j )
			{
				glFaces[ i ].indices[ j ] = face->vertexOffset + data.meshVertexes[ face->meshVertexOffset + j ].offset;
				glFaces[ i ].vertices[ j ] = data.vertexes[ glFaces[ i ].indices[ j ] ];
			}
		}
		else if ( face->type == BSP_FACE_TYPE_PATCH )
		{
			// The amount of increments we need to make for each dimension, so we have the (potentially) shared points between patches
			int stepWidth = ( face->size[ 0 ] - 1 ) / 2;
			int stepHeight = ( face->size[ 1 ] - 1 ) / 2;

			int c = 0;
			for ( int k = 0; k < face->size[ 0 ]; k += stepWidth )
			{
				for ( int j = 0; j < face->size[ 1 ]; j += stepHeight )
				{
					int index = face->vertexOffset + j * face->size[ 0 ] + k;
					glFaces[ i ].indices.push_back( index );
					glFaces[ i ].controlPoints[ c++ ] = &data.vertexes[ index ];	
				}
			}
		}
		else
		{
			continue;
		}

		const shaderInfo_t* shader = GetShaderInfo( i );

		// Perform tessellation, if requested.
		if ( shader && shader->tessSize != 0.0f && shader->deformCmd != VERTEXDEFORM_FUNC_UNDEFINED )
		{
			/*
			deformModel_t* def = new deformModel_t();

			// Tessellate each triangle
			for ( size_t j = 0; j < glFaces[ i ].indices.size(); j += 3 )
			{
				//const bspVertex_t& a = data.vertexes[ glFaces[ i ].indices[ j ] ];
				//const bspVertex_t& b = data.vertexes[ glFaces[ i ].indices[ j + 1 ] ];
				//const bspVertex_t& c = data.vertexes[ glFaces[ i ].indices[ j + 2 ] ];
					
				//TessellateTri( def->vertices, def->tris, shader->tessSize, a, b, c );
			}

			// Grab all of the vertices produced by every tessellation and do the deform
			for ( bspVertex_t& vertex: def->vertices )
			{
				vertex = LGenDeformVertex( vertex, shader );
			}

			def->vbo = GenBufferObject( GL_ARRAY_BUFFER, sizeof( bspVertex_t ) * def->vertices.size(), &def->vertices[ 0 ], GL_STATIC_DRAW );
			def->ibo = GenBufferObject( GL_ELEMENT_ARRAY_BUFFER, sizeof( triangle_t ) * def->tris.size(), &def->tris[ 0 ].indices[ 0 ], GL_STATIC_DRAW );

			glDeformed[ i ] = def;
			*/
		}
	}
}

void Q3BspMap::ReadFile( const std::string& filepath, const int scale )
{
	// Open file, verify it if we succeed
    FILE* file = fopen( filepath.c_str(), "rb" );

    if ( !file )
    {
        MLOG_ERROR("Failed to open %s\n", filepath.c_str() );
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
}

void Q3BspMap::GenNonShaderTextures( uint32_t loadFlags )
{
	GLint oldAlign;
	GL_CHECK( glGetIntegerv( GL_UNPACK_ALIGNMENT, &oldAlign ) );
	GL_CHECK( glPixelStorei( GL_UNPACK_ALIGNMENT, 1 ) );

	// This is just a hack to brute force load assets without taking into account the effect shader files.
	// Now, find and generate the textures. We first start with the image files.
	glTextures.resize( data.numTextures, 0 );
	GL_CHECK( glGenTextures( glTextures.size(), &glTextures[ 0 ] ) );

	glSamplers.resize( data.numTextures, 0 );
	GL_CHECK( glGenSamplers( glSamplers.size(), &glSamplers[ 0 ] ) );

	static const char* validImgExt[] = 
	{
		".jpg", ".png", ".tga", ".tiff", ".bmp"
	};

//	const std::string& root = filepath.substr( 0, filepath.find_last_of( '/' ) ) + "/../";

	for ( int t = 0; t < data.numTextures; t++ )
	{
		bool success = false;

		std::string fname( data.textures[ t ].name );

		const std::string& texPath = data.basePath + fname;
		
		// If we don't have a file extension appended in the name,
		// try to find one for it which is valid
		if ( fname.find_last_of( '.' ) == std::string::npos )
		{
			for ( int i = 0; i < SIGNED_LEN( validImgExt ); ++i )
			{
				const std::string& str = texPath + std::string( validImgExt[ i ] );

				if ( LoadTextureFromFile( str.c_str(), glTextures[ t ], glSamplers[ t ], loadFlags, GL_REPEAT ) )
				{
					success = true;
					break;
				}
			}
		}
		
		// Stub out the texture for this iteration by continue; warn user
		if ( !success )
			goto FAIL_WARN;

		continue;

FAIL_WARN:
		MLOG_WARNING( "Could not find a file extension for \'%s\'", texPath.c_str() );
		GL_CHECK( glDeleteTextures( 1, &glTextures[ t ] ) );
		glTextures[ t ] = 0;
		glSamplers[ t ] = 0;
	}

	GL_CHECK( glPixelStorei( GL_UNPACK_ALIGNMENT, oldAlign ) );

	// And then generate all of the lightmaps
	glLightmaps.resize( data.numLightmaps, 0 );
	GL_CHECK( glGenTextures( glLightmaps.size(), &glLightmaps[ 0 ] ) );
	GL_CHECK( glGenSamplers( 1, &glLightmapSampler ) );

	GL_CHECK( glSamplerParameteri( glLightmapSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR ) );
	GL_CHECK( glSamplerParameteri( glLightmapSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR ) );
	GL_CHECK( glSamplerParameteri( glLightmapSampler, GL_TEXTURE_WRAP_S, GL_REPEAT ) );
	GL_CHECK( glSamplerParameteri( glLightmapSampler, GL_TEXTURE_WRAP_T, GL_REPEAT ) );

	for ( int l = 0; l < data.numLightmaps; ++l )
	{	
		GL_CHECK( glBindTexture( GL_TEXTURE_2D, glLightmaps[ l ] ) );

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

	GL_CHECK( glBindTexture( GL_TEXTURE_2D, 0 ) );
}

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

bool Q3BspMap::IsClusterVisible( int sourceCluster, int testCluster )
{
    if ( !data.visdata->bitsets || ( sourceCluster < 0 ) )
        return true;

    int i = ( sourceCluster * data.visdata->sizeVector ) + ( testCluster >> 3 );

    byte visSet = data.visdata->bitsets[ i ];

    return ( visSet & ( 1 << ( testCluster & 7 ) ) ) != 0;
}

