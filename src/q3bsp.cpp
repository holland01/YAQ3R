#include "q3bsp.h"
#include "aabb.h"
#include "io.h"
#include "effect_shader.h"
#include "lib/cstring_util.h"
#include "worker/wapi.h"

using namespace std;

//-------------------------------------------------------------------------------
// Data tweaking
//-------------------------------------------------------------------------------

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
// Read Event Handling
//-------------------------------------------------------------------------------

static int gBspDesc = -1;

using q3BspAllocFn_t =
	std::function< void( char* received, mapData_t& data, int length ) >;

static std::array< q3BspAllocFn_t, BSP_NUM_ENTRIES > gBspAllocTable =
{{
	[]( char* received, mapData_t& data, int length )
	{
		data.entitiesSrc.resize( length / sizeof( char ) );
		data.entityStringLen = data.entitiesSrc.size();
		memcpy( &data.entitiesSrc[ 0 ], received, length );
		data.entities.infoString = &data.entitiesSrc[ 0 ];
	},

	[]( char* received, mapData_t& data, int length )
	{
		data.shaders.resize( length / sizeof( bspShader_t ) );
		memcpy( &data.shaders[ 0 ], received, length );
		data.numShaders = data.shaders.size();
	},

	[]( char* received, mapData_t& data, int length )
	{
		data.planes.resize( length / sizeof( bspPlane_t ) );
		memcpy( &data.planes[ 0 ], received, length );
		data.numPlanes = data.planes.size();
	},

	[]( char* received, mapData_t& data, int length )
	{
		data.nodes.resize( length / sizeof( bspNode_t ) );
		memcpy( &data.nodes[ 0 ], received, length );
		data.numNodes = data.nodes.size();
	},

	[]( char* received, mapData_t& data, int length )
	{
		data.leaves.resize( length / sizeof( bspLeaf_t ) );
		memcpy( &data.leaves[ 0 ], received, length );
		data.numLeaves = data.leaves.size();
	},

	[]( char* received, mapData_t& data, int length )
	{
		data.leafFaces.resize( length / sizeof( bspLeafFace_t ) );
		memcpy( &data.leafFaces[ 0 ], received, length );
		data.numLeafFaces = data.leafFaces.size();
	},

	[]( char* received, mapData_t& data, int length )
	{
		data.leafBrushes.resize( length / sizeof( bspLeafBrush_t ) );
		memcpy( &data.leafBrushes[ 0 ], received, length );
		data.numLeafBrushes = data.leafBrushes.size();
	},

	[]( char* received, mapData_t& data, int length )
	{
		data.models.resize( length / sizeof( bspModel_t ) );
		memcpy( &data.models[ 0 ], received, length );
		data.numModels = data.models.size();
	},

	[]( char* received, mapData_t& data, int length )
	{
		data.brushes.resize( length / sizeof( bspBrush_t ) );
		memcpy( &data.brushes[ 0 ], received, length );
		data.numBrushes = data.brushes.size();
	},

	[]( char* received, mapData_t& data, int length )
	{
		data.brushSides.resize( length / sizeof( bspBrushSide_t ) );
		memcpy( &data.brushSides[ 0 ], received, length );
	    data.numBrushSides = data.brushSides.size();
	},

	[]( char* received, mapData_t& data, int length )
	{
		data.vertexes.resize( length / sizeof( bspVertex_t ) );
		memcpy( &data.vertexes[ 0 ], received, length );
		data.numVertexes = data.vertexes.size();
	},

	[]( char* received, mapData_t& data, int length )
	{
		data.meshVertexes.resize( length / sizeof( bspMeshVertex_t ) );
		memcpy( &data.meshVertexes[ 0 ], received, length );
		data.numMeshVertexes = data.meshVertexes.size();
	},

	[]( char* received, mapData_t& data, int length )
	{
		data.fogs.resize( length / sizeof( bspFog_t ) );
		memcpy( &data.fogs[ 0 ], received, length );
		data.numFogs = data.fogs.size();
	},

	[]( char* received, mapData_t& data, int length )
	{
		data.faces.resize( length / sizeof( bspShader_t ) );
		memcpy( &data.faces[ 0 ], received, length );
		data.numFaces = data.faces.size();
	},

	[]( char* received, mapData_t& data, int length )
	{
		data.lightmaps.resize( length / sizeof( bspLightmap_t ) );
		memcpy( &data.lightmaps[ 0 ], received, length );
		data.numLightmaps = data.lightmaps.size();
	},

	[]( char* received, mapData_t& data, int length )
	{
		data.lightvols.resize( length / sizeof( bspLightvol_t ) );
		memcpy( &data.lightvols[ 0 ], received, length );
		data.numLightvols = data.lightvols.size();
	},

	[]( char* received, mapData_t& data, int length )
	{
		data.bitsetSrc.resize( length / sizeof( unsigned char ) );
		memcpy( &data.bitsetSrc[ 0 ], received, length );
		data.visdata.bitsets = &data.bitsetSrc[ 0 ];
		data.numVisdataVecs = length;
	}
}};

static void ReadChunk( char* data, int size, void* param );

static INLINE void SendRequest( wApiChunkInfo_t& info, void* param )
{
	gFileWebWorker.Await( ReadChunk, "ReadFile_Chunk",
		( char* )&info, sizeof( info ), param );
}

static void ReadFin( Q3BspMap* map )
{
	// swizzle coordinates from left-handed Z UP axis to right-handed Y UP axis.
	// Also scale anything as necessary (or desired)

	for ( size_t i = 0; i < map->data.nodes.size(); ++i )
	{
		ScaleCoords( map->data.nodes[ i ].boxMax, map->GetScaleFactor() );
		ScaleCoords( map->data.nodes[ i ].boxMin, map->GetScaleFactor() );

		SwizzleCoords( map->data.nodes[ i ].boxMax );
		SwizzleCoords( map->data.nodes[ i ].boxMin );
	}

	for ( size_t i = 0; i < map->data.leaves.size(); ++i )
	{
		ScaleCoords( map->data.leaves[ i ].boxMax, map->GetScaleFactor() );
		ScaleCoords( map->data.leaves[ i ].boxMin, map->GetScaleFactor() );

		SwizzleCoords( map->data.leaves[ i ].boxMax );
		SwizzleCoords( map->data.leaves[ i ].boxMin );
	}

	for ( size_t i = 0; i < map->data.planes.size(); ++i )
	{
		map->data.planes[ i ].distance *= map->GetScaleFactor();
		ScaleCoords( map->data.planes[ i ].normal, ( float ) map->GetScaleFactor() );
		SwizzleCoords( map->data.planes[ i ].normal );
	}

	for ( size_t i = 0; i < map->data.vertexes.size(); ++i )
	{
		ScaleCoords( map->data.vertexes[ i ].texCoords[ 0 ],
			( float ) map->GetScaleFactor() );
		ScaleCoords( map->data.vertexes[ i ].texCoords[ 1 ],
			( float ) map->GetScaleFactor() );

		ScaleCoords( map->data.vertexes[ i ].normal,
			( float ) map->GetScaleFactor() );
		ScaleCoords( map->data.vertexes[ i ].position,
			( float ) map->GetScaleFactor() );

		SwizzleCoords( map->data.vertexes[ i ].position );
		SwizzleCoords( map->data.vertexes[ i ].normal );
	}

	for ( size_t i = 0; i < map->data.models.size(); ++i )
	{
		ScaleCoords( map->data.models[ i ].boxMax,
			( float ) map->GetScaleFactor() );
		ScaleCoords( map->data.models[ i ].boxMin,
			( float ) map->GetScaleFactor() );

		SwizzleCoords( map->data.models[ i ].boxMax );
		SwizzleCoords( map->data.models[ i ].boxMin );
	}

	for ( size_t i = 0; i < map->data.faces.size(); ++i )
	{
		bspFace_t& face = map->data.faces[ i ];

		ScaleCoords( face.normal, ( float ) map->GetScaleFactor() );
		ScaleCoords( face.lightmapOrigin, ( float ) map->GetScaleFactor() );
		ScaleCoords( face.lightmapStVecs[ 0 ], ( float ) map->GetScaleFactor() );
		ScaleCoords( face.lightmapStVecs[ 1 ], ( float ) map->GetScaleFactor() );

		SwizzleCoords( face.normal );
		SwizzleCoords( face.lightmapOrigin );
		SwizzleCoords( face.lightmapStVecs[ 0 ] );
		SwizzleCoords( face.lightmapStVecs[ 1 ] );
	}

	S_LoadShaders( map );
}

static void ReadChunk( char* data, int size, void* param )
{
	MLOG_INFO( "Entering ReadChunk..." );

	if ( !data )
	{
		MLOG_ERROR( "Null data received; bailing..." );
		return;
	}
	if ( !param )
	{
		MLOG_ERROR( "Null param received; bailing..." );
		return;
	}

	Q3BspMap* map = ( Q3BspMap* )param;

	MLOG_INFO( "Data Received: %s\n Size of data received: %i", data, size );

	switch ( gBspDesc )
	{
		// Header received; validate and then send it off...
		case -1:
			MLOG_INFO( "Validating...." );
			memcpy( &map->data.header, data, size );
			if ( !map->Validate() )
			{
				MLOG_ERROR( "BSP Map \'%s\' is invalid.", map->GetFileName().c_str() );
				return;
			}
			MLOG_INFO( "Validation successful" );
		// Grab any remaining lumps we need...
		default:
			// Check to see if the data received here is from a previous
			// directory entry
			if ( gBspDesc >= 0 )
			{
				gBspAllocTable[ gBspDesc ]( data, map->data, size );
			}
			MLOG_INFO( "Fall through; gBspDesc = %i", gBspDesc );
			// Do we have any more requests to make?
			if ( ++gBspDesc < ( int ) BSP_NUM_ENTRIES )
			{
				wApiChunkInfo_t info;
				info.offset = map->data.header.directories[ gBspDesc ].offset;
				info.size = map->data.header.directories[ gBspDesc ].length;
				SendRequest( info, param );
			}
			else
			{
				gBspDesc = -1;
				ReadFin( map );
			}
			break;
	}
}

static void ReadBegin( char* data, int size, void* param )
{
	int* result = ( int* )data;
	if ( !result || !( *result ) )
	{
		MLOG_ERROR( "Bailing out; Worker ReadFile_Begin failed." );
	}

	MLOG_INFO( "Beginning header query..." );
	wApiChunkInfo_t info;
	info.offset = 0;
	info.size = sizeof( bspHeader_t );
	SendRequest( info, param );
}

//-------------------------------------------------------------------------------
// Q3BspMap
//-------------------------------------------------------------------------------
Q3BspMap::Q3BspMap( void )
	 :	scaleFactor( 1 ),
		mapAllocated( false ),
		readFinishEvent( nullptr ),
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
	const bspFace_t& face = data.faces[ faceIndex ];
	const shaderInfo_t* shader = nullptr;

	if ( face.shader != -1 )
	{
		shader = GetShaderInfo( data.shaders[ face.shader ].name );
	}

	if ( face.fog != -1 && !shader )
	{
		shader = GetShaderInfo( data.fogs[ face.fog ].name );
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
		data.nodes.clear();
		data.leaves.clear();
		data.leafBrushes.clear();
		data.leafFaces.clear();
		data.planes.clear();
		data.vertexes.clear();
		data.brushes.clear();
		data.brushSides.clear();
		data.shaders.clear();
		data.models.clear();
		data.fogs.clear();
		data.faces.clear();
		data.meshVertexes.clear();
		data.lightmaps.clear();
		data.lightvols.clear();
		data.bitsetSrc.clear();
		data.entitiesSrc.clear();
		mapAllocated = false;
	}
}

void Q3BspMap::Read( const std::string& filepath, int scale,
	onReadFinish_t finishCallback )
{
	if ( IsAllocated() )
		DestroyMap();

	readFinishEvent = finishCallback;
	scaleFactor = scale;
	name = File_StripExt( File_StripPath( filepath ) );
	data.basePath = filepath.substr( 0, filepath.find_last_of( '/' ) ) + "/../";
	File_QueryAsync( filepath, ReadBegin, this );
}

mapEntity_t Q3BspMap::GetFirstSpawnPoint( void ) const
{
	mapEntity_t ret;

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
			mem = ( unsigned char* ) data.entities.infoString;
			numBytes = data.header.directories[ BSP_LUMP_ENTITIES ].length;
			break;

		default:
			return;
	}

	f = fopen( filepath.c_str(), "wb" );
	fprintf( f, "%s\n", mem );
}

bool Q3BspMap::Validate( void )
{
	if ( data.header.id[ 0 ] != 'I' || data.header.id[ 1 ] != 'B'
		|| data.header.id[ 2 ] != 'S' || data.header.id[ 3 ] != 'P' )
	{
		MLOG_WARNING( "Header ID does NOT match \'IBSP\'. ID read is: %s \n",
			data.header.id );
		return false;
	}

	if ( data.header.version != BSP_Q3_VERSION )
	{
		MLOG_WARNING( "Header version does NOT match %i. Version found is %i\n",
			BSP_Q3_VERSION, data.header.version );
		return false;
	}

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
	if ( !data.visdata.bitsets || ( sourceCluster < 0 ) )
	{
		return true;
	}

	int i = ( sourceCluster * data.visdata.sizeVector ) + ( testCluster >> 3 );

	unsigned char visSet = data.visdata.bitsets[ i ];

	return ( visSet & ( 1 << ( testCluster & 7 ) ) ) != 0;
}
