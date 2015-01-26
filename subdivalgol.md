Overview
==========

```
void BaryToWorld( float world[ 3 ], const float bary[ 3 ], const float vertices[ 3 ][ 3 ] )
{
	world[ 0 ] = bary[ 0 ] * vertices[ 0 ][ 0 ] + bary[ 1 ] * vertices[ 1 ][ 0 ] + bary[ 2 ] * vertices[ 2 ][ 0 ];
	world[ 1 ] = bary[ 0 ] * vertices[ 0 ][ 1 ] + bary[ 1 ] * vertices[ 1 ][ 1 ] + bary[ 2 ] * vertices[ 2 ][ 1 ];
	world[ 2 ] = bary[ 0 ] * vertices[ 0 ][ 2 ] + bary[ 1 ] * vertices[ 1 ][ 2 ] + bary[ 2 ] * vertices[ 2 ][ 2 ];		
}

void SubDivide( triList_t* list, const float vertices[ 3 ][ 3 ], const int level, const int max )
{
	if (level == max)
		return;

	// Find the best fit subdivision location; this is performed by 
	// comparing each vertex of the triangle with the two lengths
	// of the potential subdivision edge it faces. The distance
	// of the half edge which is the largest, opposite of one of the 
	// vertices (the corresponding of which will become the origin of the dividing line), is what 
	// we want.

	float maxLength = 0.0f;
	float opposing[ 3 ] = {};
	int subdivOriginIndex = 0;

	// 'i' in this context denotes the vertex which is a candidate for the subdivision line
	for ( int i = 0; i < 3; ++i )
	{
		// Barycentric indices for the point opposite to our current vertex
		int a = ( i + 1 ) % 3;
		int b = ( i + 2 ) % 3;

		float baryOpposite[ 3 ] = {};
		baryOpposite[ a ] = 0.5f;
		baryOpposite[ b ] = 0.5f;

		float worldOpposite[ 3 ];
		BaryToWorld( worldOpposite, baryOpposite, vertices );
		
		// We only need one vector, since the distances on both sides
		// are equal
		float baryA[ 3 ] = {};
		baryA[ a ] = 1.0f;

		float worldA[ 3 ];
		BaryToWorld( worldA, baryA, vertices );

		// Compute a direction from the opposite point to our 'A' vertex
		float oppositeToA[ 3 ];
		Vec3Sub( oppositeToA, worldA, worldOpposite );

		float oppLen = Vec3Length( oppositeToA )
		if ( oppLen > maxLength )
		{
			memcpy( opposing, worldOpposite, sizeof( float ) * 3 );
			subdivOriginIndex = i;
			maxLength = oppLen;
		}
	}

	// TODO: finish subdiv'ving
}
```