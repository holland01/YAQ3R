#pragma once

#include "common.h"
#include "renderer/util.h"
#include "aabb.h"

void MapModelGenIndexBuffer( gIndexBuffer_t& buffer );

class Q3BspMap;
struct bspVertex_t;
struct mapPatch_t;

struct mapModel_t
{
	static const size_t INDEX_SIZE = 4u;

	bool						deform: 1;
	GLuint						vboOffset;
	intptr_t					iboOffset;
	GLsizei						iboRange; // num indices being drawn - may very well be something other than indices.size(), so we choose not to rely on it
	int32_t						subdivLevel;

	gIndexBuffer_t				indices; // NOTE: these _will_ be cleared if the instance is actually a mapPatch_t underneath, since there's a much more useful data structure for that purpose...

	AABB						bounds;

	mapModel_t( void );
	virtual ~mapModel_t( void );

	void								EncloseBoundsOnPoint( const glm::vec3& v );

	virtual void						CalcBounds( const mapData_t& data );		

	virtual	void						Generate( std::vector< bspVertex_t >& vertexData, 
												  const Q3BspMap* map, 
												  size_t faceOffset );

	mapPatch_t*							ToPatch( void );

	const mapPatch_t*					ToPatch( void ) const;
};

struct mapPatch_t : public mapModel_t
{
	std::vector< const bspVertex_t* >	controlPoints; // control point elems are stored in multiples of 9
	std::vector< bspVertex_t >			patchVertices;
	guBufferOffsetList_t				rowIndices;
	guBufferRangeList_t					trisPerRow;

	mapPatch_t( void );

	void						CalcBounds( const mapData_t& data ) override;		

	void						Generate( std::vector< bspVertex_t >& vertexData, 
												  const Q3BspMap* map, 
												  size_t faceOffset ) override;
};

INLINE mapPatch_t*	mapModel_t::ToPatch( void )
{
	return ( mapPatch_t* )this;
}

INLINE const mapPatch_t*	mapModel_t::ToPatch( void ) const
{
	return ( const mapPatch_t* )this;
}