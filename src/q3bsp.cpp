#include "q3bsp.h"
#include "aabb.h"
#include "io.h"
#include "effect_shader.h"
#include "lib/cstring_util.h"

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

const shaderInfo_t* Q3BspMap::GetShaderInfo( const char* name ) const
{
	auto it = effectShaders.find( name );

	if ( it != effectShaders.end() )
	{
		/*
		glslMade is only true if there's been shader compiles; this shouldn't be here
		*/
		//if ( !it->second.glslMade )
			//return nullptr;



		return &it->second;
	}

	return nullptr;
}

const shaderInfo_t* Q3BspMap::GetShaderInfo( int faceIndex ) const
{
	const bspFace_t* face = data.faces + faceIndex;
	const shaderInfo_t* shader = nullptr;

	if ( face->shader != -1 )
	{
		shader = GetShaderInfo( data.shaders[ face->shader ].name );
	}

	if ( face->fog != -1 && !shader )
	{
		shader = GetShaderInfo( data.fogs[ face->fog ].name );
	}

	return shader;
}

bool Q3BspMap::IsMapOnlyShader( const std::string& shaderPath ) const
{
	const std::string shadername( File_StripExt( File_StripPath( shaderPath ) ) );

	return shadername == name;
}

void Q3BspMap::DestroyMap( void )
{
	if ( mapAllocated )
	{
		delete[] data.visdata->bitsets;
		delete[] data.buffer;

		mapAllocated = false;
	}
}

mapEntity_t Q3BspMap::Read( const std::string& filepath, const int scale )
{
	if ( IsAllocated() )
		DestroyMap();

	mapEntity_t ret;

	data.basePath = filepath.substr( 0, filepath.find_last_of( '/' ) ) + "/../";

	if ( !ReadFile( filepath, scale ) )
	{
		return ret;
	}

	mapAllocated = true;

	const char* pInfo = data.entities.infoString;

	while ( *pInfo )
	{
		char tok[ 64 ];
		memset( tok, 0, sizeof( tok ) );

		pInfo = StrReadToken( &tok[ 0 ], pInfo );

		if ( strcmp( tok, "{" ) == 0 )
		{
			memset( tok, 0, sizeof( tok ) );
			pInfo = StrReadToken( tok, pInfo );

			glm::vec3 origin;
			bool found = false;

			while ( strcmp( tok, "}" ) != 0 )
			{
				if ( strcmp( tok, "\"classname\"" ) == 0 )
				{
					char newTok[ 64 ];
					memset( newTok, 0, sizeof( newTok ) );
					pInfo = StrReadToken( newTok, pInfo );
					if ( strcmp( newTok, "\"info_player_deathmatch\"" ) == 0
						 || strcmp( newTok, "\"info_player_start\"" ) == 0 )
					{
						found = true;
					}

					goto end_iteration;
				}

				if ( strcmp( tok, "\"origin\"" ) == 0 )
				{
					pInfo = StrNextNumber( pInfo );

					origin[ 0 ] = StrReadFloat( pInfo );
					origin[ 1 ] = StrReadFloat( pInfo );
					origin[ 2 ] = StrReadFloat( pInfo );
				}

end_iteration:
				memset( tok, 0, sizeof( tok ) );
				pInfo = StrReadToken( tok, pInfo );
			}

			if ( found )
			{
				ret.origin = origin;
				SwizzleCoords( ret.origin );
				break;
			}
		}
	}

	return ret;
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

bool Q3BspMap::Validate( void )
{
	if ( !data.buffer )
	{
		return false;
	}

	data.header = ( bspHeader_t* )data.buffer;

	if ( data.header->id[ 0 ] != 'I' || data.header->id[ 1 ] != 'B'
		|| data.header->id[ 2 ] != 'S' || data.header->id[ 3 ] != 'P' )
	{
		MLOG_ERROR( "Header ID does NOT match \'IBSP\'. ID read is: %s \n",
			data.header->id );
		return false;
	}

	if ( data.header->version != BSP_Q3_VERSION )
	{
		MLOG_ERROR( "Header version does NOT match %i. Version found is %i\n",
			BSP_Q3_VERSION, data.header->version );
		return false;
	}

	return true;
}

static void Q3BspMap_ReadChunk( char* data, int size, void* param )
{
	MLOG_INFO( "Data Received: %s\n Size of data received: %i", data, size );
}


static void Q3BspMap_ReadBegin( char* data, int size, void* param )
{
	char bogus[] = "0x1337FEED";

	gFileWebWorker.Await( Q3BspMap_ReadChunk, "ReadFile_Chunk", bogus,
		strlen( bogus ), param );
}


bool Q3BspMap::ReadFile( const std::string& filepath, const int scale )
{
	// TODO: refactor data.buffer into a std::vector,
	// and replace redundant file processing code (in the case that EM_USE_WORKER_THREAD)
	// isn't defined with File_Open/File_GetData (we're doing that only) if worker threads
	// are used right *now* because it's far simpler to test and time is short.
#ifdef EM_USE_WORKER_THREAD
	{
		File_QueryAsync( filepath, Q3BspMap_ReadBegin, this );
		return false;
	}
#else
	// Open file, verify it if we succeed
	{
		FILE* file = File_Open( filepath );
		if ( !file )
		{
			return false;
		}

		fseek( file, 0, SEEK_END );
		size_t fsize = ftell( file );
		data.buffer = new byte[ fsize ]();
		fseek( file, 0, SEEK_SET );
		fread( data.buffer, fsize, 1, file );
		rewind( file );

		if ( !Validate() )
		{
			return false;
		}

		data.visdata = ( bspVisdata_t* )( data.buffer +
			data.header->directories[ BSP_LUMP_VISDATA ].offset );
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
#endif

	//
	// Read map data
	//
	data.entities.infoString = ( char* )( data.buffer + data.header->directories[ BSP_LUMP_ENTITIES ].offset );
	data.entityStringLen = data.header->directories[ BSP_LUMP_ENTITIES ].length / sizeof( char );

	data.shaders = ( bspShader_t* )( data.buffer + data.header->directories[ BSP_LUMP_SHADERS ].offset );
	data.numShaders = data.header->directories[ BSP_LUMP_SHADERS ].length / sizeof( bspShader_t );

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

	data.fogs = ( bspFog_t* )( data.buffer + data.header->directories[ BSP_LUMP_FOGS ].offset );
	data.numFogs = data.header->directories[ BSP_LUMP_FOGS ].length / sizeof( bspFog_t );

	data.lightmaps = ( bspLightmap_t* )( data.buffer + data.header->directories[ BSP_LUMP_LIGHTMAPS ].offset );
	data.numLightmaps = data.header->directories[ BSP_LUMP_LIGHTMAPS ].length / sizeof( bspLightmap_t );

	data.lightvols = ( bspLightvol_t* )( data.buffer + data.header->directories[ BSP_LUMP_LIGHTVOLS ].offset );
	data.numLightvols = data.header->directories[ BSP_LUMP_LIGHTVOLS ].length / sizeof( bspLightvol_t );

	data.brushes = ( bspBrush_t* )( data.buffer + data.header->directories[ BSP_LUMP_BRUSHES ].offset );
	data.numBrushes = data.header->directories[ BSP_LUMP_BRUSHES ].length / sizeof( bspBrush_t );

	data.brushSides = ( bspBrushSide_t* )( data.buffer + data.header->directories[ BSP_LUMP_BRUSH_SIDES ].offset );
	data.numBrushSides = data.header->directories[ BSP_LUMP_BRUSH_SIDES ].length / sizeof( bspBrushSide_t );

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

#ifndef EM_USE_WORKER_THREAD
	LogBSPData( BSP_LUMP_SHADERS, ( void* ) data.shaders, data.numShaders );
	LogBSPData( BSP_LUMP_FOGS, ( void* ) ( data.fogs ), data.numFogs );
	LogBSPData( BSP_LUMP_ENTITIES, ( void *) ( data.entities.infoString ), -1 );
#endif

	name = File_StripExt( File_StripPath( filepath ) );

	return true;
}

bspLeaf_t* Q3BspMap::FindClosestLeaf( const glm::vec3& camPos )
{
	int nodeIndex = 0;

	uint32_t count = 0;
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

		count++;
	}

	nodeIndex = -( nodeIndex + 1 );

	return  &data.leaves[ nodeIndex ];
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
