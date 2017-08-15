#pragma once

#include "common.h"
#include "deform.h"
#include "bsp_data.h"
#include <memory>
#include <unordered_map>
#include <unordered_set>

struct shaderInfo_t;
struct renderPayload_t;

struct mapEntity_t
{
	glm::vec3 origin;
	std::string className;
};

struct gPathMap_t;

using shaderList_t = std::vector< const shaderInfo_t * >;

#define Q3BSPMAP_DEFAULT_SHADER_NAME "noshader"

// The data-driven mechanism which is used to map a stage
// to a texture index relies on the texture path itself
// to use as a key into an unordered hash map. Consequently, 
// only one actual stage will be written to unless a different approach is taken.

// Other stages which need the same path are likely to exist, 
// but won't be assigned a corresponding index for that texture in the atlas.
// So, a linked list is a simple alternative.
struct pathLinkNode_t
{
	shaderStage_t* stage = nullptr;
	pathLinkNode_t* next = nullptr;
};

class Q3BspMap
{
private:

	Q3BspMap( const Q3BspMap& ) = delete;
	Q3BspMap& operator=( Q3BspMap ) = delete;

	int									scaleFactor;

	int 								defaultShaderIndex;

	bool								mapAllocated;

	std::string							name;

	std::stack< pathLinkNode_t* > 		pathLinkRoots;

	void 						MakeStagePathList( pathLinkNode_t * node );

public:
	std::unique_ptr< renderPayload_t > 	payload;

	onFinishEvent_t						readFinishEvent;

	std::unordered_map< std::string, shaderInfo_t > effectShaders;

	shaderList_t 									opaqueShaderList;
	shaderList_t 									transparentShaderList;

	Q3BspMap( void );
	~Q3BspMap( void );

	mapData_t					data;

	void 						AddEffectShader( shaderInfo_t effectShader );

	void 						OnShaderReadFinish( void );


	static void 				OnShaderLoadImagesFinish( void* param );


	static void					OnMainLoadImagesFinish( void* param );

	// retrives the first spawn point found in the text file.
	mapEntity_t					GetFirstSpawnPoint( void ) const;

	void 						GenerateProgramListFromShaders( void );

	const shaderInfo_t*			GetDefaultEffectShader( void ) const { return &effectShaders.at( Q3BSPMAP_DEFAULT_SHADER_NAME ); }

	std::vector< gPathMap_t > 	GetShaderSourcesList( void );

	const shaderInfo_t*			GetShaderInfo( const char* name ) const;

	const shaderInfo_t*			GetShaderInfo( int faceIndex ) const;

	const std::string&			GetFileName( void ) const { return name; }

	std::string 				GetBinLayoutString( void ) const;

	std::string					GetPrintString( const std::string& title = "." ) const;

	int							GetScaleFactor( void ) const
									{ return scaleFactor; }

	bool						Validate( void );

	void						ZeroData( void );

	void						Read( const std::string& filepath, int scale,
	 								onFinishEvent_t finishCallback );

	void						WriteLumpToFile( uint32_t lump );

	bspLeaf_t*					FindClosestLeaf( const glm::vec3& camPos );

	void 						MakeAllocated( void )
									{ mapAllocated = true; }

	bool 						IsDefaultShader( const shaderInfo_t* info ) const;

	bool 						IsShaderUsed( shaderInfo_t* outInfo ) const;

	bool						IsClusterVisible( int sourceCluster, int testCluster );

	bool						IsAllocated( void ) const
								{ return mapAllocated; }

	bool						IsMapOnlyShader( const std::string& filepath ) const;

	bool 						IsTransparentShader( const shaderInfo_t * scriptShader ) const;

	bool 						IsNoDrawShader( const shaderInfo_t * scriptShader ) const;

	void						DestroyMap( void );



	friend class BSPRenderer;
};

// Debug only: will be no-op if called in release build.
// These are used to verify that each name found in
// mapData_t::shaders doesn't belong to both
// categories (main - no shader entries, or actual shader entries)
void Q3BspMapTest_ShaderNameTagMain( const char* name );
void Q3BspMapTest_ShaderNameTagShader( const char* name );
void Q3BspMapTest_ShaderNameRun( void );

void Q3Bsp_SwizzleCoords( glm::vec3& v );
void Q3Bsp_SwizzleCoords( glm::ivec3& v );
void Q3Bsp_SwizzleCoords( glm::vec2& v );

template < typename T >
static INLINE bool Q3Bsp_MatchShaderInfoFromName( const std::vector< T >& list, const char* name, int& toWrite )
{
	int listLength = ( int ) list.size();

	for ( int i = 0; i < listLength; ++i )
	{
		if ( strncmp( &list[ i ].name[ 0 ], name, BSP_MAX_SHADER_TOKEN_LENGTH - 1 ) == 0 )
		{
		 	toWrite = i;
		 	return true;
		}
	}

	return false;
}