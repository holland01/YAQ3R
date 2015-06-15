#pragma once

#include "common.h"
#include "deform.h"
#include "bsp_data.h"
#include "glutil.h"
#include <memory>

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
    bspFace_t*          faces;
	
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

	texture_t						glDummyTexture;

	void							ReadFile( const std::string& filepath, const int scale );
	
	void							GenNonShaderTextures( uint32_t loadFlags );
	
	void							GenRenderData( void );

	void							GenTextureRGB8( texture_t t );

public:

	std::vector< texture_t >		glTextures;
	std::vector< texture_t >		glLightmaps;	// textures - has one->one map with lightmap indices
//	std::vector< texture_t >		glCubes;		// has one->one map with leaf nodes
	std::vector< mapModel_t >		glFaces;		// has one->one map with face indices
					GLuint			glLightmapSampler;

	std::map< std::string, shaderInfo_t > effectShaders;

	glm::vec3						lightvolGrid;

    Q3BspMap( void );
    ~Q3BspMap( void );

	mapData_t					data;

    void						Read( const std::string& filepath, const int scale, uint32_t loadFlags );

	void						WriteLumpToFile( uint32_t lump );

    void						SetVertexColorIf( bool ( predicate )( unsigned char* ), const glm::u8vec3& rgbColor );

    bspLeaf_t*					FindClosestLeaf( const glm::vec3& camPos );

	bool						IsTransFace( int faceIndex ) const;

    bool						IsClusterVisible( int sourceCluster, int testCluster );

    bool						IsAllocated( void ) const { return mapAllocated; }

	const shaderInfo_t*			GetShaderInfo( int faceIndex ) const;

	const texture_t&			GetDummyTexture( void ) const { return glDummyTexture; }

    void						DestroyMap( void );
};
