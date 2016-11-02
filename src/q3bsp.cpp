#include "q3bsp.h"
#include "aabb.h"
#include "io.h"
#include "effect_shader.h"
#include "renderer/util.h"
#include "lib/cstring_util.h"
#include "lib/async_image_io.h"
#include "worker/wapi.h"
#include "renderer.h"

using namespace std;

static const Q3BspMap* gOneTrueMap = nullptr;

//------------------------------------------------------------------------------
// Data tweaking
//------------------------------------------------------------------------------

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

//------------------------------------------------------------------------------
// Read Event Handling
//------------------------------------------------------------------------------

static int gBspDesc = -1;

using q3BspAllocFn_t =
	std::function< void( char* received, mapData_t& data, int length ) >;

static std::array< q3BspAllocFn_t, BSP_NUM_ENTRIES > gBspAllocTable =
{{
	// BSP_LUMP_ENTITIES = 0x00
	[]( char* received, mapData_t& data, int length )
	{
		data.entitiesSrc.resize( length / sizeof( char ) );
		data.entityStringLen = data.entitiesSrc.size();
		memcpy( &data.entitiesSrc[ 0 ], received, length );
		data.entities.infoString = &data.entitiesSrc[ 0 ];
	},

	// BSP_LUMP_SHADERS = 0x01
	[]( char* received, mapData_t& data, int length )
	{
		data.shaders.resize( length / sizeof( bspShader_t ) );
		memcpy( &data.shaders[ 0 ], received, length );
		data.numShaders = data.shaders.size();
	},

	// BSP_LUMP_PLANES = 0x02
	[]( char* received, mapData_t& data, int length )
	{
		data.planes.resize( length / sizeof( bspPlane_t ) );
		memcpy( &data.planes[ 0 ], received, length );
		data.numPlanes = data.planes.size();
	},

	// BSP_LUMP_NODES = 0x03
	[]( char* received, mapData_t& data, int length )
	{
		data.nodes.resize( length / sizeof( bspNode_t ) );
		memcpy( &data.nodes[ 0 ], received, length );
		data.numNodes = data.nodes.size();
	},

	// BSP_LUMP_LEAVES = 0x04
	[]( char* received, mapData_t& data, int length )
	{
		data.leaves.resize( length / sizeof( bspLeaf_t ) );
		memcpy( &data.leaves[ 0 ], received, length );
		data.numLeaves = data.leaves.size();
	},

	// BSP_LUMP_LEAF_FACES = 0x05
	[]( char* received, mapData_t& data, int length )
	{
		data.leafFaces.resize( length / sizeof( bspLeafFace_t ) );
		memcpy( &data.leafFaces[ 0 ], received, length );
		data.numLeafFaces = data.leafFaces.size();
	},

	// BSP_LUMP_LEAF_BRUSHES = 0x06
	[]( char* received, mapData_t& data, int length )
	{
		data.leafBrushes.resize( length / sizeof( bspLeafBrush_t ) );
		memcpy( &data.leafBrushes[ 0 ], received, length );
		data.numLeafBrushes = data.leafBrushes.size();
	},

	// BSP_LUMP_MODELS = 0x07
	[]( char* received, mapData_t& data, int length )
	{
		data.models.resize( length / sizeof( bspModel_t ) );
		memcpy( &data.models[ 0 ], received, length );
		data.numModels = data.models.size();
	},

	// BSP_LUMP_BRUSHES = 0x08
	[]( char* received, mapData_t& data, int length )
	{
		data.brushes.resize( length / sizeof( bspBrush_t ) );
		memcpy( &data.brushes[ 0 ], received, length );
		data.numBrushes = data.brushes.size();
	},

	// BSP_LUMP_BRUSH_SIDES = 0x09
	[]( char* received, mapData_t& data, int length )
	{
		data.brushSides.resize( length / sizeof( bspBrushSide_t ) );
		memcpy( &data.brushSides[ 0 ], received, length );
	    data.numBrushSides = data.brushSides.size();
	},

	// BSP_LUMP_VERTEXES = 0x0A
	[]( char* received, mapData_t& data, int length )
	{
		data.vertexes.resize( length / sizeof( bspVertex_t ) );
		memcpy( &data.vertexes[ 0 ], received, length );
		data.numVertexes = data.vertexes.size();
	},

	// BSP_LUMP_MESH_VERTEXES = 0x0B
	[]( char* received, mapData_t& data, int length )
	{
		data.meshVertexes.resize( length / sizeof( bspMeshVertex_t ) );
		memcpy( &data.meshVertexes[ 0 ], received, length );
		data.numMeshVertexes = data.meshVertexes.size();
	},

	// BSP_LUMP_FOGS = 0x0C
	[]( char* received, mapData_t& data, int length )
	{
		data.fogs.resize( length / sizeof( bspFog_t ) );
		memcpy( &data.fogs[ 0 ], received, length );
		data.numFogs = data.fogs.size();
	},

	// BSP_LUMP_FACES = 0x0D
	[]( char* received, mapData_t& data, int length )
	{
		data.faces.resize( length / sizeof( bspFace_t ) );
		memcpy( &data.faces[ 0 ], received, length );
		data.numFaces = data.faces.size();
	},

	// BSP_LUMP_LIGHTMAPS = 0x0E
	[]( char* received, mapData_t& data, int length )
	{
		data.lightmaps.resize( length / sizeof( bspLightmap_t ) );
		memcpy( &data.lightmaps[ 0 ], received, length );
		data.numLightmaps = data.lightmaps.size();
	},

	// BSP_LUMP_LIGHTVOLS = 0x0F
	[]( char* received, mapData_t& data, int length )
	{
		data.lightvols.resize( length / sizeof( bspLightvol_t ) );
		memcpy( &data.lightvols[ 0 ], received, length );
		data.numLightvols = data.lightvols.size();
	},

	// BSP_LUMP_VISDATA = 0x10
	[]( char* received, mapData_t& data, int length )
	{
		memcpy( &data.visdata, received, sizeof( bspVisdata_t ) );
		data.bitsetSrc.resize( data.visdata.numVectors *
			data.visdata.sizeVector, 0 );
		memcpy( &data.bitsetSrc[ 0 ], received + sizeof( bspVisdata_t ),
		 	length - sizeof( bspVisdata_t ) );
		data.numVisdataVecs = length;
	}
}};

static void ReadChunk( char* data, int size, void* param );

static INLINE void SendRequest( wApiChunkInfo_t& info, void* param )
{
	gFileWebWorker.Await( ReadChunk, "ReadFile_Chunk",
		( char* )&info, sizeof( info ), param );
}

static INLINE void MapReadFin_UnmountFin( char* data, int size, void* param )
{
	UNUSED( data );
	UNUSED( size );

	Q3BspMap* map = ( Q3BspMap* ) param;

	if ( !map )
	{
		MLOG_ERROR( "Map ptr received is NULL!" );
		return;
	}

	S_LoadShaders( map );
}

static void MapReadFin( Q3BspMap* map )
{
	// Swizzle coordinates from left-handed Z UP axis
	// to right-handed Y UP axis.
	// Also perform scaling, desired

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
		ScaleCoords( map->data.planes[ i ].normal,
			( float ) map->GetScaleFactor() );
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
		ScaleCoords( face.lightmapStVecs[ 0 ],
			( float ) map->GetScaleFactor() );
		ScaleCoords( face.lightmapStVecs[ 1 ],
			( float ) map->GetScaleFactor() );

		SwizzleCoords( face.normal );
		SwizzleCoords( face.lightmapOrigin );
		SwizzleCoords( face.lightmapStVecs[ 0 ] );
		SwizzleCoords( face.lightmapStVecs[ 1 ] );
	}

	gFileWebWorker.Await( MapReadFin_UnmountFin,  "UnmountPackages",
			NULL, 0, map );
}

static void ReadChunk( char* data, int size, void* param )
{
	if ( !data )
	{
		MLOG_ERROR( "%s", "Null data received; bailing..." );
		return;
	}

	if ( !param )
	{
		MLOG_ERROR( "%s", "Null param received; bailing..." );
		return;
	}

	Q3BspMap* map = ( Q3BspMap* ) param;

	switch ( gBspDesc )
	{
		// We've just begun, so we validate the header first
		case -1:
			memcpy( &map->data.header, data, sizeof( map->data.header ) );
			if ( !map->Validate() )
			{
				MLOG_ERROR( "BSP Map \'%s\' is invalid.",
					map->GetFileName().c_str() );
				return;
			}

		// We don't break on purpose above, because we want
		// to initiate a fetch for the first chunk immediately after
		default:

			// We're ready to take what was received from the
			// previous chunk read; -1 implies that
			// we have nothing, since we've only
			// just validated the header
			if ( gBspDesc >= 0 && size )
			{
				gBspAllocTable[ gBspDesc ]( data, map->data, size );
			}

			if ( ++gBspDesc < ( int ) BSP_NUM_ENTRIES )
			{
				wApiChunkInfo_t info;

				info.offset = map->data.header.directories[ gBspDesc ].offset;
				info.size = map->data.header.directories[ gBspDesc ].length;

				if ( info.size )
				{
					SendRequest( info, map );
				}
				// Avoid wasting time by moving to the next
				// directory if we have nothing to read here
				else
				{
					ReadChunk( data, 0, map );
				}
			}
			else
			{
				gBspDesc = -1;
				MapReadFin( map );
			}
			break;
	}
}

static void ReadBegin( char* data, int size, void* param )
{
	UNUSED( size );

	if ( !data )
	{
		MLOG_ERROR( "Bailing out; Worker ReadFile_Begin failed." );
		return;
	}

	wApiChunkInfo_t info;
	info.offset = 0;
	info.size = sizeof( bspHeader_t );
	SendRequest( info, param );
}

static INLINE void LoadImagesFinish( gImageParamList_t& dest,
	gImageLoadTracker_t** imageTracker )
{
	dest = std::move( ( *imageTracker )->textures );

	delete *imageTracker;
	*imageTracker = nullptr;
}

static void UnmountShadersFin( char* data, int size,
		void* arg )
{
	UNUSED( data );
	UNUSED( size );

	MLOG_INFO( "===============\n"\
		"Loading images from effect shaders..."\
		"\n===============" );

	GU_LoadShaderTextures( *( ( Q3BspMap* ) arg ), GMakeSampler() );
}

//------------------------------------------------------------------------------
// Q3BspMap
//------------------------------------------------------------------------------
Q3BspMap::Q3BspMap( void )
	 :	scaleFactor( 1 ),
		mapAllocated( false ),
		payload( nullptr ),
		readFinishEvent( nullptr )
{
	gOneTrueMap = this;
	ZeroData();
}

Q3BspMap::~Q3BspMap( void )
{
	DestroyMap();
}

void Q3BspMap::OnShaderReadFinish( void )
{
	gFileWebWorker.Await( UnmountShadersFin,
		"UnmountPackages", NULL, 0, this );
}

void Q3BspMap::OnShaderLoadImagesFinish( void* param )
{
	gImageLoadTracker_t** imageTracker = ( gImageLoadTracker_t** ) param;
	Q3BspMap& map = ( *imageTracker )->map;
	map.payload.reset( new renderPayload_t() );
	map.payload->sampler = ( *imageTracker )->sampler;

	LoadImagesFinish( map.payload->shaderImages, imageTracker );
	GU_LoadMainTextures( map, map.payload->sampler );
}

void Q3BspMap::OnMainLoadImagesFinish( void* param )
{
	gImageLoadTracker_t** imageTracker = ( gImageLoadTracker_t** ) param;
	Q3BspMap& map = ( *imageTracker )->map;
	LoadImagesFinish( map.payload->mainImages, imageTracker );

	puts( "Main images finished." );
	map.readFinishEvent( &map );
}

const shaderInfo_t* Q3BspMap::GetShaderInfo( const char* name ) const
{
	auto it = effectShaders.find( name );

	if ( it != effectShaders.end() )
	{
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
	const std::string shadername(
		File_StripExt( File_StripPath( shaderPath ) ) );

	return shadername == name;
}

void Q3BspMap::ZeroData( void )
{
	memset( ( uint8_t* ) &data.entities, 0,
		sizeof( data ) - offsetof( mapData_t, entities ) );

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
}

void Q3BspMap::DestroyMap( void )
{
	if ( mapAllocated )
	{
		ZeroData();
		mapAllocated = false;
	}
}

void Q3BspMap::Read( const std::string& filepath, int scale,
	onFinishEvent_t finishCallback )
{
	if ( IsAllocated() )
	{
		DestroyMap();
	}

	readFinishEvent = finishCallback;
	scaleFactor = scale;
	name = File_StripExt( File_StripPath( filepath ) );

	std::string readParams( "maps|" );
	readParams.append( filepath );

	gFileWebWorker.Await( ReadBegin, "ReadFile_Begin", readParams, this );
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
		// then our needed camera data is
		// in a leaf somewhere in front of this node,
		// otherwise it's behind the node somewhere.
		glm::vec3 planeNormal( plane->normal.x, plane->normal.y,
			plane->normal.z );

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
	if ( data.bitsetSrc.empty() || ( sourceCluster < 0 ) )
	{
		return true;
	}

	int i = ( sourceCluster * data.visdata.sizeVector ) + ( testCluster >> 3 );

	unsigned char visSet = data.bitsetSrc[ i ];

	return ( visSet & ( 1 << ( testCluster & 7 ) ) ) != 0;
}

std::string Q3BspMap::GetPrintString( const std::string& title ) const
{
	std::stringstream ss;

	ss << "[" << title << "]" << "\n"
	<< "\tscaleFactor: " << scaleFactor << "\n"
	<< "\tmapAllocated: " << mapAllocated << "\n"
	<< "\tpayload: " << std::hex << ( uintptr_t ) payload.get() << "\n"
	<< "\tname: " << name << "\n"
	<< "\treadFinishEvent: "
		<< std::hex << ( uintptr_t ) readFinishEvent
		<< "\n"
	<< "\teffectShaders.size() " << std::dec << effectShaders.size() << "\n"
	<< "\t[data]:\n"
	<< "\t\theader.id: " << data.header.id << "\n"
	<< "\t\tshaders.size(): " << data.shaders.size() << "\n"
	<< "\t\tplanes.size(): " <<  data.planes.size() << "\n"
	<< "\t\tnodes.size(): " << data.nodes.size() << "\n"
	<< "\t\tleaves.size(): " << data.leaves.size() << "\n"
	<< "\t\tleafFaces.size(): " << data.leafFaces.size() << "\n"
	<< "\t\tmodels.size(): " << data.models.size() << "\n"
	<< "\t\tbrushes.size(): " << data.brushes.size() << "\n"
	<< "\t\tbrushSides.size(): " << data.brushSides.size() << "\n"
	<< "\t\tvertexes.size(): " << data.vertexes.size() << "\n"
	<< "\t\tmeshVertexes.size(): " << data.meshVertexes.size() << "\n"
	<< "\t\tfogs.size(): " << data.fogs.size() << "\n"
	<< "\t\tfaces.size(): " << data.faces.size() << "\n"
	<< "\t\tlightmaps.size(): " << data.lightmaps.size() << "\n"
	<< "\t\tlightvols.size(): " << data.lightvols.size() << "\n";

	return ss.str();
}

void Q3BspMap::AssertTrueMap( void ) const
{
	if ( this != gOneTrueMap )
	{
		MLOG_ERROR( "Big fucking problem." );
	}
}
