#pragma once

#include "common.h"
#include "deform.h"
#include "bsp_data.h"
#include <memory>
#include <unordered_map>

struct shaderInfo_t;
struct renderPayload_t;

struct mapEntity_t
{
	glm::vec3 origin;
	std::string className;
};

class Q3BspMap
{
private:

	Q3BspMap( const Q3BspMap& ) = delete;
	Q3BspMap& operator=( Q3BspMap ) = delete;

	int									scaleFactor;

	bool								mapAllocated;

	std::unique_ptr< renderPayload_t > 	payload;

	std::string							name;

public:

	onFinishEvent_t					readFinishEvent;

	std::unordered_map< std::string, shaderInfo_t > effectShaders;

	Q3BspMap( void );
	~Q3BspMap( void );

	mapData_t					data;

	void 						OnShaderReadFinish( void );

	static void 				OnShaderLoadImagesFinish( void* param );


	static void					OnMainLoadImagesFinish( void* param );
	
	// retrives the first spawn point found in the text file.
	mapEntity_t					GetFirstSpawnPoint( void ) const;

	bool						Validate( void );

	void						Read( const std::string& filepath, int scale,
	 								onFinishEvent_t finishCallback );

	void						WriteLumpToFile( uint32_t lump );

	bspLeaf_t*					FindClosestLeaf( const glm::vec3& camPos );

	bool						IsClusterVisible( int sourceCluster, int testCluster );

	bool						IsAllocated( void ) const { return mapAllocated; }

	void 						MakeAllocated( void ) { mapAllocated = true; }

	const shaderInfo_t*			GetShaderInfo( const char* name ) const;

	const shaderInfo_t*			GetShaderInfo( int faceIndex ) const;

	const std::string&			GetFileName( void ) const { return name; }

	int							GetScaleFactor( void ) const { return scaleFactor; }

	bool						IsMapOnlyShader( const std::string& filepath ) const;

	void						DestroyMap( void );
};
