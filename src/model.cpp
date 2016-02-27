#include "model.h"
#include "q3bsp.h"

static gIndexBuffer_t* gServiceIndexBuffer = nullptr;

void MapModelGenIndexBuffer( gIndexBuffer_t& buffer )
{
	gServiceIndexBuffer = &buffer;
}

mapModel_t::mapModel_t( void )
	: deform( false ),
	  vboOffset( 0 ),
	  iboOffset( 0 ),
	  iboRange( 0 ),
	  subdivLevel( 0 )
{
}

void mapModel_t::Generate( std::vector< bspVertex_t >& vertexData, 
						  const Q3BspMap* map, 
						  size_t faceOffset )
{
	const bspFace_t* face = &map->data.faces[ faceOffset ];
	iboRange = face->numMeshVertexes;

	iboOffset = ( intptr_t )( gServiceIndexBuffer? gServiceIndexBuffer->size(): 0 );
	vboOffset = ( GLuint ) vertexData.size();

	vertexData.resize( vertexData.size() + face->numMeshVertexes );

	for ( int32_t j = 0; j < face->numMeshVertexes; ++j )
	{
		uint32_t index = face->vertexOffset + map->data.meshVertexes[ face->meshVertexOffset + j ].offset;

		if ( gServiceIndexBuffer )
			gServiceIndexBuffer->push_back( index );
		else
			indices.push_back( index );
	}
}

mapModel_t::~mapModel_t( void )
{
}

void mapModel_t::EncloseBoundsOnPoint( const glm::vec3& v )
{
	if ( v.x < bounds.minPoint.x ) bounds.minPoint.x = v.x;
	if ( v.y < bounds.minPoint.y ) bounds.minPoint.y = v.y;

	if ( v.x > bounds.maxPoint.x ) bounds.maxPoint.x = v.x;
	if ( v.y > bounds.maxPoint.y ) bounds.maxPoint.y = v.y;

#ifdef AABB_MAX_Z_LESS_THAN_MIN_Z
	if ( v.z > bounds.minPoint.z ) bounds.minPoint.z = v.z;
#else
	if ( v.z < bounds.maxPoint.z ) bounds.maxPoint.z = v.z;
#endif
}

void mapModel_t::CalcBounds( const mapData_t& data )
{
	bounds.Empty();
	
	for ( size_t i = 0; i < indices.size(); ++i )
	{
		EncloseBoundsOnPoint( data.vertexes[ indices[ i ] ].position );
	}
}

//-----------------------------------------------------------------

mapPatch_t::mapPatch_t( void )
{
}

void mapPatch_t::Generate(  std::vector< bspVertex_t >& vertexData, 
							const Q3BspMap* map, 
							size_t faceOffset )
{
	const bspFace_t* face = &map->data.faces[ faceOffset ];
	const shaderInfo_t* shader = map->GetShaderInfo( faceOffset );

	iboOffset = ( intptr_t )( gServiceIndexBuffer? gServiceIndexBuffer->size(): 0 );
	vboOffset = ( GLuint ) vertexData.size();

	int width = ( face->patchDimensions[ 0 ] - 1 ) / 2;
	int height = ( face->patchDimensions[ 1 ] - 1 ) / 2;

	// ( k, j ) maps to a ( row, col ) index scheme referring to the beginning of a patch
	int n, m;
	controlPoints.resize( width * height * 9 );

	// buffer holds potentially ALL indices for each model, so we need to subtract the base size after patch generation

	size_t store = 0;
	if ( store )
		store = gServiceIndexBuffer->size();

	for ( n = 0; n < width; ++n )
	{
		for ( m = 0; m < height; ++m )
		{
			int baseSource = face->vertexOffset + 2 * m * width + 2 * n;
			int baseDest = ( m * width + n ) * 9;

			for ( int32_t c = 0; c < 3; ++c )
			{
				controlPoints[ baseDest + c * 3 + 0 ] = &map->data.vertexes[ baseSource + c * face->patchDimensions[ 0 ] + 0 ];
				controlPoints[ baseDest + c * 3 + 1 ] = &map->data.vertexes[ baseSource + c * face->patchDimensions[ 0 ] + 1 ];
				controlPoints[ baseDest + c * 3 + 2 ] = &map->data.vertexes[ baseSource + c * face->patchDimensions[ 0 ] + 2 ];
			}

			if ( gServiceIndexBuffer )
			{
				GenPatch( *gServiceIndexBuffer, 
					this, shader, baseDest, ( int32_t ) vertexData.size() );
			}
			else
			{
				GenPatch( indices,
					this, shader, baseDest, ( int32_t ) vertexData.size() );
			}
		}
	}

	// Snag the amount which is relevant to our portion
	if ( gServiceIndexBuffer )
		iboRange = gServiceIndexBuffer->size() - store;
	else
		iboRange = indices.size();

	// Calculate our row offsets; there are width * height patches, and our subdivision
	// level represents the amount of tessellation rows for each patch, every row consisting
	// of 2 * (subdivlevel + 1) vertices

	const uint32_t L1 = subdivLevel + 1;
	rowIndices.resize( width * height * subdivLevel, 0 );
	
	// This will always be 2 * L1 for every row, since they're all uniform.
	// Best thing to is be aware of what the largest number of row indices
	// is out of all of the mapPatch_t's generated, and then create a data store (preallocated as the size of the largest number of row indices)
	// which can be written to with the appropriate value (trisPerRow would then 
	// just be the scalar 2 * L1

	// (OR just use the scalar value in glDrawElements; you could write a separate GU_MultiDrawElements which
	// is designed to take N index buffer offsets, each of which has a range of the same size).
	
	trisPerRow.resize( width * height * subdivLevel, 2 * L1 );

	for ( size_t y = 0; y < rowIndices.size(); ++y )
	{
		rowIndices[ y ] = iboOffset + y * 2 * L1;
	}

	vertexData.insert( vertexData.end(), patchVertices.begin(), patchVertices.end() );
}

void mapPatch_t::CalcBounds( const mapData_t& data )
{
	UNUSED( data );

	bounds.Empty();

	for ( const bspVertex_t& v: patchVertices )
	{
		EncloseBoundsOnPoint( v.position );
	}
}