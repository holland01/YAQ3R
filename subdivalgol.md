Overview
==========

**Note: this is all pseudo code. A few structs are designed for brevity's sake, but we can use our imaginations for anything else which is undefined :D**

```
struct vec3_t
{
	float v[ 3 ]; 
};

struct tri_t
{
	int indices[ 3 ];
};

struct triList_t
{
	tri_t* triangles;
	int numTriangles;

	vertexBuffer_t vertices;
};

void BaryToWorld( vec3_t* world, const float bary[ 3 ], const int* indices, const vec3_t* vertices )
{
	world->v[ 0 ] = bary[ 0 ] * vertices[ indices[ 0 ] ].v[ 0 ] + bary[ 1 ] * vertices[ indices[ 1 ] ].v[ 0 ] + bary[ 2 ] * vertices[ indices[ 2 ] ].v[ 0 ];
	world->v[ 1 ] = bary[ 0 ] * vertices[ indices[ 0 ] ].v[ 1 ] + bary[ 1 ] * vertices[ indices[ 1 ] ].v[ 1 ] + bary[ 2 ] * vertices[ indices[ 2 ] ].v[ 1 ];
	world->v[ 2 ] = bary[ 0 ] * vertices[ indices[ 0 ] ].v[ 2 ] + bary[ 1 ] * vertices[ indices[ 1 ] ].v[ 2 ] + bary[ 2 ] * vertices[ indices[ 2 ] ].v[ 2 ];		
}

void SubDivide( triList_t* list, const tri_t* candidate, const vertexBuffer_t* vertices, const int level, const int max )
{
	if (level == max)
	{
		return;
	}

	// Find the best fit subdivision location; this is performed by 
	// comparing each vertex of the triangle with the two lengths
	// of the potential subdivision edge it faces. The distance
	// of the half edge which is the largest, opposite of one of the 
	// vertices (the corresponding of which will become the origin of the dividing line), is what 
	// we want.

	// !NOTE: winding order for front-faces must be CCW. Always.

	float maxLength = 0.0f;
	
	int oIndex = 0; // origin
	int aIndex = 0; // index after origin, in counter-clockwise terms
	int bIndex = 0; // index afer a, in counter-clockwise terms
	
	vec3_t subend; // The end of the dividing line; opposite to the vertex and residing on the (relative to the vertex) "base" of the triangle
	
	// 'i' in this context denotes the vertex which is a candidate for the subdivision line
	for ( int i = 0; i < 3; ++i )
	{
		// Barycentric indices for the point opposite to our current vertex
		int a = ( i + 1 ) % 3;
		int b = ( i + 2 ) % 3;

		float baryOpposite[ 3 ] = {};
		baryOpposite[ a ] = 0.5f;
		baryOpposite[ b ] = 0.5f;

		vec3_t worldOpposite;
		BaryToWorld( &worldOpposite, baryOpposite, candidate->indices, vertices->data );
		
		// Compute a direction from our 'origin' vertex to the opposite point
		vec3_t originToSurface;
		Vec3Sub( &originToSurface, &worldOpposite, &vertices->data[ candidate->indices[ i ] ] );

		float lineLength = Vec3Length( &originToSurface )
		
		if ( lineLength > maxLength )
		{
			memcpy( &subend, &worldOpposite, sizeof( vec3_t ) );
			
			oIndex = i;			
			aIndex = a;
			bIndex = b;

			maxLength = lineLength;
		}
	}

	// Perform the subdivision here. 

	// We use separate vertex buffers for each subdivision
	// because only the max level candidates are added to the list,
	// and using separate buffers prevents nastiness with maintenance.
	
	// The initial vertex buffer is likely to come from a source which uses
	// thousands ( or more ) of vertices. So, we take this into account from the get-go
	// and tie it all together by utilizing the same vertex/index structure as the first buffer.
 
	vertexBuffer_t newBuffer;

	VertexBuffer_Add( &newBuffer, vertices->data[ candidate->indices[ 0 ] ] );
	VertexBuffer_Add( &newBuffer, vertices->data[ candidate->indices[ 1 ] ] );
	VertexBuffer_Add( &newBuffer, vertices->data[ candidate->indices[ 2 ] ] );
	VertexBuffer_Add( &newBuffer, &subend );

	tri_t t1 = 
	{
		{
			oIndex, 
			aIndex,
			3 
		}
	};

	SubDivide( list, &t1, &newBuffer, level + 1, max );

	tri_t t2 = 
	{
		{
			3,
			bIndex,
			oIndex
		}
	};

	SubDivide( list, &t2, &newBuffer, level + 1, max );

	// Are we done? If so, we need to merge with our list
	if ( ( level + 1 ) == max )
	{
		// The buffer and tri list append operations both perform copies, 
		// so we need not worry about the fact that the t1, t2, and newBuffer are allocated on the stack...

		int triIndexBase = list->vertices.length - 1;

		VertexBuffer_Append( &list->vertices, &newBuffer );

		for ( int i = 0; i < 3; ++i )
		{
			t1.indices[ i ] += triIndexBase;
			t2.indices[ i ] += triIndexBase;
		}

		TriList_Add( &list->vertices, &t1 );
		TriList_Add( &list->vertices, &t2 );
	}
}
```