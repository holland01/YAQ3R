#pragma once

#include "common.h"
#include "vec.h"
#include "q3m_model.h"
#include <array>

#define BEZ_BUF_COUNT 2
#define BEZ_CONTROL_POINT_COUNT 9

#define DEFORM_TABLE_SIZE 1024
#define DEFORM_TABLE_SIZE_LOG_2 10
#define DEFORM_TABLE_MASK ( DEFORM_TABLE_SIZE - 1 )

#define DEFORM_CALC_TABLE( table, base, offset, t, f, a ) \
	( ( base ) + ( table )[ int ( ( offset ) + ( t ) * ( f ) * DEFORM_TABLE_SIZE ) & DEFORM_TABLE_MASK ] * ( a ) )

struct bspVertex_t;
struct bspFace_t;
struct shaderInfo_t;
struct deformModel_t;
struct mapModel_t;
struct mapData_t;

struct deformGlobal_t
{
	std::array< float, DEFORM_TABLE_SIZE > sinTable;
	std::array< float, DEFORM_TABLE_SIZE > triTable;
};

extern deformGlobal_t deformCache;

float GenDeformScale( const glm::vec3& position, const shaderInfo_t* shader );

void GenPatch( bezPatch_t* patch, const shaderInfo_t* shader );

void TessellateTri( 
	std::vector< bspVertex_t >& outVerts, 
	std::vector< triangle_t >& triIndices,
	float amount,
	float normalOffsetScale,	// where vertex = vertex + ( normal * normalOffsetScale )
	const bspVertex_t& a, 
	const bspVertex_t& b, 
	const bspVertex_t& c
);
