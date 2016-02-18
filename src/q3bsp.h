#pragma once

#include "common.h"
#include "deform.h"
#include "bsp_data.h"
#include "circle_buffer.h"
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

	bspTexture_t*       textures;
	bspModel_t*         models;

	bspEffect_t*		effectShaders;

	bspFace_t*			faces;

	bspMeshVertex_t*    meshVertexes;

	bspLightmap_t*		lightmaps;
	bspLightvol_t*		lightvols;

	bspVisdata_t*       visdata;

	int                 entityStringLen;
	int                 numEffectShaders;

	int                 numNodes;

	int                 numLeaves;
	int                 numLeafFaces;
	int					numLeafBrushes;

	int                 numPlanes;

	int                 numVertexes;

	int					numBrushes;
	int					numBrushSides;

	int                 numTextures;
	int                 numModels;

	int					numEffects;
	int                 numFaces;

	int                 numMeshVertexes;

	int					numLightmaps;
	int					numLightvols;

	int                 numVisdataVecs;

	std::string			basePath; // root directory of the map
};

class Q3BspMap
{
private:

	Q3BspMap( const Q3BspMap& ) = delete;
	Q3BspMap& operator=( Q3BspMap ) = delete;

	bool							mapAllocated;

	void							ReadFile( const std::string& filepath, const int scale );

public:

	std::unordered_map< std::string, shaderInfo_t > effectShaders;

	Q3BspMap( void );
	~Q3BspMap( void );

	mapData_t					data;

	void						Read( const std::string& filepath, const int scale );

	void						WriteLumpToFile( uint32_t lump );

	bspLeaf_t*					FindClosestLeaf( const glm::vec3& camPos );

	bool						IsClusterVisible( int sourceCluster, int testCluster );

	bool						IsAllocated( void ) const { return mapAllocated; }

	const shaderInfo_t*			GetShaderInfo( int faceIndex ) const;

	void						DestroyMap( void );
};
