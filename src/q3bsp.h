#pragma once

#include "common.h"
#include "deform.h"
#include "bsp_data.h"
#include <memory>
#include <unordered_map>

struct shaderInfo_t;

struct mapData_t
{
	byte*				buffer;  // all file memory comes from this

	bspHeader_t*		header;

	bspEntity_t         entities;

	bspNode_t*          nodes;

	bspLeaf_t*          leaves;
	bspLeafBrush_t*		leafBrushes;
	bspLeafFace_t*      leafFaces;

	bspPlane_t*         planes;
	bspVertex_t*        vertexes;

	bspBrush_t*			brushes;
	bspBrushSide_t*		brushSides;

	bspShader_t*		shaders;
	bspModel_t*         models;

	bspFog_t*			fogs;

	bspFace_t*			faces;

	bspMeshVertex_t*    meshVertexes;

	bspLightmap_t*		lightmaps;
	bspLightvol_t*		lightvols;

	bspVisdata_t*       visdata;

	int                 entityStringLen;

	int                 numNodes;

	int                 numLeaves;
	int                 numLeafFaces;
	int					numLeafBrushes;

	int                 numPlanes;

	int                 numVertexes;

	int					numBrushes;
	int					numBrushSides;

	int                 numShaders;
	int                 numModels;

	int                 numFogs;
	int                 numFaces;

	int                 numMeshVertexes;

	int					numLightmaps;
	int					numLightvols;

	int                 numVisdataVecs;

	std::string			basePath; // root directory of the map
};

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

	bool							mapAllocated;

	std::string						name;

	void							ReadFile( const std::string& filepath, const int scale );

public:

	std::unordered_map< std::string, shaderInfo_t > effectShaders;

	Q3BspMap( void );
	~Q3BspMap( void );

	mapData_t					data;

	mapEntity_t					Read( const std::string& filepath, const int scale );

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
