#version 420

smooth in vec3 frag_Position;
smooth in vec3 frag_Normal;

uniform sampler2D fragRadianceSampler;
uniform vec4 fragTargetPlane;
uniform vec3 fragSurfaceNormal;
uniform vec2 fragMin; // xz
uniform vec2 fragMax; // xz

layout( location = 0 ) out vec4 fragment;

const float gamma = 1.0 / 2.2;

void main( void )
{
	// u: horizontal axis
	// v: vertical axis
	// w: origin
	vec2 u, v, w;
	if ( fragMax.y < fragMin.y )
	{
		w = vec2( fragMin.x, fragMax.y );
		u = fragMax - w;
		v = fragMin - w;
	}
	else
	{
		w = fragMin;
		u = vec2( fragMax.x, fragMin.y ) - w;
		v = vec2( fragMin.x, fragMax.y ) - w;
	}

	vec3 prevRayDir = vec3( 0.0 );
	
	vec3 planeNormal = fragTargetPlane.xyz;

	float distanceNumerator = fragTargetPlane.w - dot( r.origin, planeNormal );
	float invULen = 1.0 / length( u );
	float invVLen = 1.0 / length( v );

	vec3 normal = normalize( frag_Normal );

	vec4 color = vec4( 0.0 );
	for ( float phi = 0.0; phi <= 1.5707; phi += 0.1 )
	{
		for ( float theta = 0.0; theta <= 3.14159; theta += 0.1 )
		{
			vec3 rayDir = vec3( cos( theta ) * cos( phi ), sin( phi ), cos( phi ) * sin( theta ) );
			float cosAngRay = dot( rayDir, planeNormal );

			if ( cosAngRay == 0.0 )
			{
				continue;
			}
			
			// Compute distance from origin to sample point
			float t = distanceNumerator / cosAngRay; 

			// Otherwise, continue the use of the integration function: http://www.cs.virginia.edu/~jdl/bib/appearance/analytic%20models/schlick94b.pdf
			vec2 worldSample = (frag_Position + rayDir * t).xz;

			if ( worldSample.x < fragMin.x || worldSample.x > fragMax.x )
			{ 
				continue;
			}

			if ( fragMax.y > fragMin.y && ( worldSample.y < fragMin.y || worldSample.y > fragMax.y ) )
			{
				continue;
			}
			else if ( fragMax.y < fragMin.y && ( worldSample.y > fragMin.y || worldSample.y < fragMax.y ) )
			{
				continue;
			}

			vec2 uvSample = worldSample - w;
			uvSample.x *= invULen;
			uvSample.y *= invVLen;

			vec3 dirNorm = normalize( rayDir );

			color += texture( fragRadianceSampler, uvSample ) * dot( normal, dirNorm ) * ( dirNorm - prevRayDir );
		
			prevRayDir = dirNorm;
		}
	}

	fragment = color;
}