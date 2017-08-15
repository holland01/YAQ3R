#include "q3bsp.h"
#include "aabb.h"
#include "io.h"
#include "effect_shader.h"
#include "renderer/util.h"
#include "lib/cstring_util.h"
#include "lib/async_image_io.h"
#include "worker/wapi.h"
#include "renderer.h"
#include "extern/gl_atlas.h"
#include "em_api.h"

using namespace std;

//------------------------------------------------------------------------------
// Internal
//------------------------------------------------------------------------------

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

static bool TestShaderFlags( 
	const shaderInfo_t * scriptShader, 
	const mapData_t & data,
	surfaceParm_t surfFlags, 
	int contentsFlags, 
	int surfaceFlags 
)
{
	if ( !scriptShader )
	{
		return false;
	}

	if ( !!( scriptShader->surfaceParms & surfFlags ) )
	{
		return true;
	}

	if ( scriptShader->mapShaderIndex != INDEX_UNDEFINED )
	{
		const bspShader_t* mapShader = &data.shaders[ scriptShader->mapShaderIndex ];

		if ( !!( mapShader->contentsFlags & contentsFlags ) )
		{
			return true;
		}

		if ( !!( mapShader->surfaceFlags & surfaceFlags ) )
		{
			return true;
		}
	}

	return false;
}

//------------------------------------------------------------------------------
// Global Functions
//------------------------------------------------------------------------------

void Q3Bsp_SwizzleCoords( glm::vec3& v )
{
	float tmp = v.y;
	v.y = v.z;
	v.z = -tmp;
}

// Straight outta copypasta
void Q3Bsp_SwizzleCoords( glm::ivec3& v )
{
	int tmp = v.y;
	v.y = v.z;
	v.z = -tmp;
}

void Q3Bsp_SwizzleCoords( glm::vec2& v )
{
	v.t = 1.0f - v.t;
}

//------------------------------------------------------------------------------
// Read Event Handling
//------------------------------------------------------------------------------

static int gBspDesc = -1;

using q3BspAllocFn_t = std::function< void( char* received, mapData_t& data,
	int length ) >;

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
	gFileWebWorker.Await( ReadChunk, "ReadMapFile_Chunk",
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

		Q3Bsp_SwizzleCoords( map->data.nodes[ i ].boxMax );
		Q3Bsp_SwizzleCoords( map->data.nodes[ i ].boxMin );
	}

	for ( size_t i = 0; i < map->data.leaves.size(); ++i )
	{
		ScaleCoords( map->data.leaves[ i ].boxMax, map->GetScaleFactor() );
		ScaleCoords( map->data.leaves[ i ].boxMin, map->GetScaleFactor() );

		Q3Bsp_SwizzleCoords( map->data.leaves[ i ].boxMax );
		Q3Bsp_SwizzleCoords( map->data.leaves[ i ].boxMin );
	}

	for ( size_t i = 0; i < map->data.planes.size(); ++i )
	{
		map->data.planes[ i ].distance *= map->GetScaleFactor();
		ScaleCoords( map->data.planes[ i ].normal,
			( float ) map->GetScaleFactor() );
		Q3Bsp_SwizzleCoords( map->data.planes[ i ].normal );
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

		Q3Bsp_SwizzleCoords( map->data.vertexes[ i ].position );
		Q3Bsp_SwizzleCoords( map->data.vertexes[ i ].normal );
		//Q3Bsp_SwizzleCoords( map->data.vertexes[ i ].texCoords[ 0 ] );
		//Q3Bsp_SwizzleCoords( map->data.vertexes[ i ].texCoords[ 1 ] );
	}

	for ( size_t i = 0; i < map->data.models.size(); ++i )
	{
		ScaleCoords( map->data.models[ i ].boxMax,
			( float ) map->GetScaleFactor() );
		ScaleCoords( map->data.models[ i ].boxMin,
			( float ) map->GetScaleFactor() );

		Q3Bsp_SwizzleCoords( map->data.models[ i ].boxMax );
		Q3Bsp_SwizzleCoords( map->data.models[ i ].boxMin );
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

		Q3Bsp_SwizzleCoords( face.normal );
		Q3Bsp_SwizzleCoords( face.lightmapOrigin );
		Q3Bsp_SwizzleCoords( face.lightmapStVecs[ 0 ] );
		Q3Bsp_SwizzleCoords( face.lightmapStVecs[ 1 ] );
	}

	for ( size_t i = 0; i < map->data.shaders.size(); ++i )
	{
		BspData_FixupAssetPath( &map->data.shaders[ i ].name[ 0 ] );

		MLOG_INFO( "[%i] %s. Content: 0x%x, Surface: 0x%x", 
			i, &map->data.shaders[ i ].name[ 0 ], map->data.shaders[ i ].contentsFlags,
			map->data.shaders[ i ].surfaceFlags );
	}

	gFileWebWorker.Await(
		MapReadFin_UnmountFin,
		"UnmountPackages",
		NULL,
		0,
		map
	);
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
				uint8_t checksum = WAPI_CalcCheckSum( data, size - 1 );

				if ( checksum != ( uint8_t ) data[ size - 1 ] )
				{
					MLOG_WARNING(
						"Bad Checksum for BSP LUMP ENTRY %x.\n"
						"Checksum sent: %x\n"
						"Checksum tested: %x",
						(uint32_t)data[size - 1],
						(uint32_t)checksum
					);
				}

				// Checksum is appended to the very end of the buffer;
				// sending the total size could cause problems
				gBspAllocTable[ gBspDesc ]( data, map->data, size - 1 );
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
		MLOG_ERROR( "Bailing out; Worker ReadMapFile_Begin failed." );
		return;
	}

	wApiChunkInfo_t info;
	info.offset = 0;
	info.size = sizeof( bspHeader_t );
	SendRequest( info, param );
}

static void UnmountShadersFin( char* data, int size, void* arg )
{
	UNUSED( data );
	UNUSED( size );

	//MLOG_INFO( "===============\n"\
	//	"Loading images from effect shaders..."\
	//	"\n===============" );

	GU_LoadShaderTextures( *( ( Q3BspMap* ) arg ) );
}

//------------------------------------------------------------------------------
// Q3BspMapTest
//------------------------------------------------------------------------------

#ifdef DEBUG
struct q3BspMapTestShaderName_t
{
	bool isShader;
	bool isMain;
};

static
std::unordered_map< std::string, q3BspMapTestShaderName_t > gTestShaderName;

#endif

void Q3BspMapTest_ShaderNameTagMain( const char* name )
{
#ifdef DEBUG
	std::string key( name );
	gTestShaderName[ key ].isMain = true;
#else
	UNUSED( name );
#endif
}

void Q3BspMapTest_ShaderNameTagShader( const char* name )
{
#ifdef DEBUG
	std::string key( name );
	gTestShaderName[ key ].isShader = true;
#else
	UNUSED( name );
#endif
}

void Q3BspMapTest_ShaderNameRun( void )
{
#ifdef DEBUG
	std::vector< std::string > names;

	for ( const auto& entry: gTestShaderName )
	{
		if ( entry.second.isMain && entry.second.isShader )
		{
			names.push_back( entry.first );
		}
	}

	if ( !names.empty() )
	{
		std::stringstream ss;

		for ( size_t i = 0; i < names.size(); ++i )
		{
			ss << "\t[" << i << "]" << names[ i ] << "\n";
		}

		MLOG_ERROR( "Found at least one misplaced or duplicated shader:\n%s",
	 		ss.str().c_str() );
	}
#endif
}

//------------------------------------------------------------------------------
// Q3BspMap
//------------------------------------------------------------------------------
Q3BspMap::Q3BspMap( void )
	 :	scaleFactor( 1 ),
	 	defaultShaderIndex( INDEX_UNDEFINED ),
		mapAllocated( false ),
		payload( nullptr ),
		readFinishEvent( nullptr )
{
	ZeroData();
}

Q3BspMap::~Q3BspMap( void )
{
	DestroyMap();
}

void Q3BspMap::AddEffectShader( shaderInfo_t effectShader )
{
	std::string key( &effectShader.name[ 0 ], strlen( &effectShader.name[ 0 ] ) );

	effectShaders.insert( shaderMapEntry_t(
		key,
		effectShader
	) );

	shaderInfo_t* afterInsert = &effectShaders[ key ];

	// Default sort is opaque; if no sort param was specified,
	// double check for any telling attributes which indicate we need to 
	// switch to the default transparent sort. 
	bool transparent = IsTransparentShader( afterInsert );

	if ( afterInsert->sort == BSP_SHADER_SORT_OPAQUE && transparent )
	{
		afterInsert->sort = BSP_SHADER_SORT_ADDITIVE;
	}

	if ( transparent )
	{
		afterInsert->sortListIndex = ( int ) transparentShaderList.size();
		transparentShaderList.push_back( afterInsert );
	}
	else
	{
		afterInsert->sortListIndex = ( int ) opaqueShaderList.size();
		opaqueShaderList.push_back( afterInsert );
	}

	// Add names and indices after entry's been copied; these are mostly
	// used for debugging.
	for ( size_t i = 0; i < afterInsert->stageBuffer.size(); ++i )
	{
		afterInsert->stageBuffer[ i ].owningBufferIndex = i;
		afterInsert->stageBuffer[ i ].owningShader = afterInsert;
	}
}

void Q3BspMap::OnShaderReadFinish( void )
{	
	// Add a default fallback shader, since there exist many faces which don't have one assigned.
	{
		shaderInfo_t noshader;

		// ctor prefills name with zeros
		strcpy( &noshader.name[ 0 ], Q3BSPMAP_DEFAULT_SHADER_NAME );

		// will find and assign appropriate indices if we have an entry: 
		// we can possibly benefit from content and surface flags.
		IsShaderUsed( &noshader );

		AddEffectShader( noshader );
	}

	auto LSortPredicate = []( const shaderInfo_t* a, const shaderInfo_t* b ) -> bool
	{
		return a->sort < b->sort;
	};

	std::sort( opaqueShaderList.begin(), opaqueShaderList.end(), LSortPredicate );
	std::sort( transparentShaderList.begin(), transparentShaderList.end(), LSortPredicate );

	printf( "opaqueShaderList Size: %u, transparentShaderList Size: %u\n", opaqueShaderList.size(),
		transparentShaderList.size() );

	// Assign indices so we have quick lookup 
	// when traversing the BSP
	for ( auto& shaderEntry: effectShaders )
	{
		const shaderList_t& list = IsTransparentShader( &shaderEntry.second ) ?
			transparentShaderList : opaqueShaderList;

		for ( int i = 0; i < ( int ) list.size(); ++i )
		{
			if ( strncmp( &list[ i ]->name[ 0 ], 
					&shaderEntry.second.name[ 0 ], 
					BSP_MAX_SHADER_TOKEN_LENGTH ) == 0 )
			{
				shaderEntry.second.sortListIndex = i;
				break;
			}
		}

		assert( shaderEntry.second.sortListIndex > INDEX_UNDEFINED );
	}

	gFileWebWorker.Await( UnmountShadersFin, "UnmountPackages", nullptr, 0, this );
}

void Q3BspMap::OnShaderLoadImagesFinish( void* param )
{
	gImageLoadTrackerPtr_t* imageTracker = ( gImageLoadTrackerPtr_t* ) param;
	Q3BspMap& map = *( ( *imageTracker )->map );

	GU_LoadMainTextures( map );
}

void Q3BspMap::OnMainLoadImagesFinish( void* param )
{
	gImageLoadTrackerPtr_t* imageTracker = ( gImageLoadTrackerPtr_t* ) param;
	Q3BspMap* map = ( *imageTracker )->map;

	Q3BspMapTest_ShaderNameRun();

	// Here we free the linked lists necessary to ensure all
	// shader stages with an image receive their correct index
	// into the atlas.
	while ( !map->pathLinkRoots.empty() )
	{
		pathLinkNode_t* node = map->pathLinkRoots.top();
		map->pathLinkRoots.pop();

		for ( pathLinkNode_t* iNode = node; iNode; )
		{
			pathLinkNode_t* tmp = iNode;
			iNode = iNode->next;
			delete tmp;
		}
	}

	map->mapAllocated = true;
	map->readFinishEvent( map );
}

const shaderInfo_t* Q3BspMap::GetShaderInfo( const char* name ) const
{
	auto it = effectShaders.find( name );

	if ( it != effectShaders.end() )
	{
		return &it->second;
	}

	return GetDefaultEffectShader();
}

const shaderInfo_t* Q3BspMap::GetShaderInfo( int faceIndex ) const
{
	const bspFace_t& face = data.faces[ faceIndex ];
	const shaderInfo_t* shader = nullptr;

	if ( face.shader < 0 || face.shader >= ( int ) data.shaders.size() )
	{
		shader = GetDefaultEffectShader();
	}
	else
	{
		shader = GetShaderInfo( data.shaders[ face.shader ].name );
	}

//	if ( face.fog != -1 && !shader )
//	{
//		shader = GetShaderInfo( data.fogs[ face.fog ].name );
//	}

	return shader;
}

void Q3BspMap::MakeStagePathList( pathLinkNode_t* node )
{
	for ( auto& entry: effectShaders )
	{
		for ( shaderStage_t& stage: entry.second.stageBuffer )
		{
			if ( stage.pathLinked )
			{
				continue;
			}

			if ( strncmp( &node->stage->texturePath[ 0 ], &stage.texturePath[ 0 ],
					BSP_MAX_SHADER_TOKEN_LENGTH ) == 0 )
			{
				stage.pathLinked = true;

				node->next = new pathLinkNode_t();
				node->next->stage = &stage;
				node = node->next;
			}
		}
	}
}

std::vector< gPathMap_t > Q3BspMap::GetShaderSourcesList( void ) 
{
	std::vector< gPathMap_t > sources;

	for ( auto& entry: effectShaders )
	{
		for ( shaderStage_t& stage: entry.second.stageBuffer )
		{
			if ( stage.pathLinked )
			{
				continue;
			}

			if ( stage.mapType == MAP_TYPE_IMAGE )
			{
				gPathMap_t initial;

				pathLinkNode_t* pathRoot = new pathLinkNode_t();
				stage.pathLinked = true;
				pathRoot->stage = &stage;

				MakeStagePathList( pathRoot );

				initial.param = pathRoot;
				initial.path = std::string( &stage.texturePath[ 0 ] );

				sources.push_back( initial );

				pathLinkRoots.push( pathRoot );
			}
		}

		Q3BspMapTest_ShaderNameTagShader( &entry.second.name[ 0 ] );
	}

	return sources;
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
				Q3Bsp_SwizzleCoords( ret.origin );
				break;
			}
		}
	}

	return ret;
}

std::string Q3BspMap::GetBinLayoutString( void ) const
{
	std::stringstream ss;

	ss << SSTREAM_BYTE_OFFSET( Q3BspMap, scaleFactor );
	ss << SSTREAM_BYTE_OFFSET( Q3BspMap, mapAllocated );

	ss << SSTREAM_BYTE_OFFSET( Q3BspMap, name );
	ss << SSTREAM_BYTE_OFFSET( Q3BspMap, payload );
	ss << SSTREAM_BYTE_OFFSET( Q3BspMap, readFinishEvent );
	ss << SSTREAM_BYTE_OFFSET( Q3BspMap, effectShaders );

	ss << SSTREAM_BYTE_OFFSET( Q3BspMap, data );

	ss << SSTREAM_BYTE_OFFSET2( mapData_t, header );

	ss << SSTREAM_BYTE_OFFSET2( mapData_t, entitiesSrc );
	ss << SSTREAM_BYTE_OFFSET2( mapData_t, shaders );
	ss << SSTREAM_BYTE_OFFSET2( mapData_t, planes );
	ss << SSTREAM_BYTE_OFFSET2( mapData_t, nodes );
	ss << SSTREAM_BYTE_OFFSET2( mapData_t, leaves );
	ss << SSTREAM_BYTE_OFFSET2( mapData_t, leafFaces );
	ss << SSTREAM_BYTE_OFFSET2( mapData_t, models );
	ss << SSTREAM_BYTE_OFFSET2( mapData_t, brushes );
	ss << SSTREAM_BYTE_OFFSET2( mapData_t, brushSides );
	ss << SSTREAM_BYTE_OFFSET2( mapData_t, vertexes );
	ss << SSTREAM_BYTE_OFFSET2( mapData_t, meshVertexes );
	ss << SSTREAM_BYTE_OFFSET2( mapData_t, fogs );
	ss << SSTREAM_BYTE_OFFSET2( mapData_t, faces );
	ss << SSTREAM_BYTE_OFFSET2( mapData_t, lightmaps );
	ss << SSTREAM_BYTE_OFFSET2( mapData_t, lightvols );
	ss << SSTREAM_BYTE_OFFSET2( mapData_t, bitsetSrc );

	ss << SSTREAM_BYTE_OFFSET2( mapData_t, entities );
	ss << SSTREAM_BYTE_OFFSET2( mapData_t, visdata );

	ss << SSTREAM_BYTE_OFFSET2( mapData_t, entityStringLen );

	ss << SSTREAM_BYTE_OFFSET2( mapData_t, numNodes );

	ss << SSTREAM_BYTE_OFFSET2( mapData_t, numLeaves );
	ss << SSTREAM_BYTE_OFFSET2( mapData_t, numLeafFaces );
	ss << SSTREAM_BYTE_OFFSET2( mapData_t, numLeafBrushes );

	ss << SSTREAM_BYTE_OFFSET2( mapData_t, numPlanes );

	ss << SSTREAM_BYTE_OFFSET2( mapData_t, numVertexes );

	ss << SSTREAM_BYTE_OFFSET2( mapData_t, numBrushes );
	ss << SSTREAM_BYTE_OFFSET2( mapData_t, numBrushSides );

	ss << SSTREAM_BYTE_OFFSET2( mapData_t, numShaders );
	ss << SSTREAM_BYTE_OFFSET2( mapData_t, numModels );

	ss << SSTREAM_BYTE_OFFSET2( mapData_t, numFogs );
	ss << SSTREAM_BYTE_OFFSET2( mapData_t, numFaces );

	ss << SSTREAM_BYTE_OFFSET2( mapData_t, numMeshVertexes );

	ss << SSTREAM_BYTE_OFFSET2( mapData_t, numLightmaps );
	ss << SSTREAM_BYTE_OFFSET2( mapData_t, numLightvols );

	ss << SSTREAM_BYTE_OFFSET2( mapData_t, numVisdataVecs );

	return ss.str();
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

bool Q3BspMap::IsDefaultShader( const shaderInfo_t* info ) const
{
	return strcmp( &info->name[ 0 ], Q3BSPMAP_DEFAULT_SHADER_NAME ) == 0;
}

bool Q3BspMap::IsShaderUsed( shaderInfo_t* outInfo ) const
{
	if ( outInfo->mapFogIndex != INDEX_UNDEFINED 
		|| outInfo->mapShaderIndex != INDEX_UNDEFINED )
	{
		return true;
	}

	Q3Bsp_MatchShaderInfoFromName< bspShader_t >( data.shaders, &outInfo->name[ 0 ], 
		outInfo->mapShaderIndex );

	//Q3Bsp_MatchShaderInfoFromName< bspFog_t >( data.fogs, &outInfo->name[ 0 ], 
	//	outInfo->mapFogIndex );

	return outInfo->mapFogIndex != INDEX_UNDEFINED 
		|| outInfo->mapShaderIndex != INDEX_UNDEFINED;
}

bool Q3BspMap::IsMapOnlyShader( const std::string& shaderPath ) const
{
	const std::string shadername(
		File_StripExt( File_StripPath( shaderPath ) ) );

	return shadername == name;
}

bool Q3BspMap::IsTransparentShader( const shaderInfo_t* scriptShader ) const
{
	if ( TestShaderFlags( 
		scriptShader, 
		data, 
		SURFPARM_TRANS | SURFPARM_WATER,
		BSP_CONTENTS_TRANSLUCENT | BSP_CONTENTS_WATER | BSP_CONTENTS_FOG, 
		0 
	) )
	{
		return true;
	}

	switch ( scriptShader->sort )
	{
		case BSP_SHADER_SORT_BANNER:
		case BSP_SHADER_SORT_UNDERWATER:
		case BSP_SHADER_SORT_ADDITIVE:
		case BSP_SHADER_SORT_NEAREST:
			return true;
		
		default:
			return false;
			break;
	}
}

bool Q3BspMap::IsNoDrawShader( const shaderInfo_t * scriptShader ) const
{
	return TestShaderFlags( 
		scriptShader, 
		data, 
		SURFPARM_NO_DRAW,
		0, 
		BSP_SURFACE_NODRAW 
	);
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

	payload.reset();

	opaqueShaderList.clear();
	transparentShaderList.clear();
	effectShaders.clear();
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

	payload.reset( new renderPayload_t() );

	for ( gla_atlas_ptr_t& atlas: payload->textureData )
	{
		atlas.reset( new gla::atlas_t() );
	}

	readFinishEvent = finishCallback;
	scaleFactor = scale;
	name = File_StripExt( File_StripPath( filepath ) );

	std::string readParams( "maps|" );
	readParams.append( filepath );

	gFileWebWorker.Await( ReadBegin, "ReadMapFile_Begin", readParams, this );
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