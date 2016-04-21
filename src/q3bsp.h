#pragma once

#include "common.h"
#include "deform.h"
#include "bsp_data.h"
#include <memory>
#include <unordered_map>

struct shaderInfo_t;

struct mapEntity_t
{
	glm::vec3 origin;
	std::string className;
};

class Q3BspMap
{
public:
	typedef void ( *onReadFinish_t )( Q3BspMap& map, void* param );

private:

	Q3BspMap( const Q3BspMap& ) = delete;
	Q3BspMap& operator=( Q3BspMap ) = delete;

	onReadFinish_t					readFinishEvent;

	void*							readFinishParam;

	bool							mapAllocated;

	std::string						name;

	bool							ReadFile( const std::string& filepath, const int scale );

public:


	std::unordered_map< std::string, shaderInfo_t > effectShaders;

	Q3BspMap( void );
	~Q3BspMap( void );

	mapData_t					data;

	bool						Validate( void );

	mapEntity_t					Read( const std::string& filepath, int scale,
	 								onReadFinish_t finishCallback, void* userParam );

	void						WriteLumpToFile( uint32_t lump );

	bspLeaf_t*					FindClosestLeaf( const glm::vec3& camPos );

	bool						IsClusterVisible( int sourceCluster, int testCluster );

	bool						IsAllocated( void ) const { return mapAllocated; }

	const shaderInfo_t*			GetShaderInfo( const char* name ) const;

	const shaderInfo_t*			GetShaderInfo( int faceIndex ) const;

	const std::string&			GetFileName( void ) const { return name; }

	bool						IsMapOnlyShader( const std::string& filepath ) const;

	void						DestroyMap( void );
};
