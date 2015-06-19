#version 450

smooth in vec3 frag_Position;
smooth in vec3 frag_Normal;

uniform sampler2D fragRadianceSampler;
uniform vec4 fragTargetPlane;
uniform vec2 fragMin; // xz
uniform vec2 fragMax; // xz

const float gamma = 1.0 / 2.2;
const float pi = 3.1415926535895;
const float piOver8 = pi / 8.0;
const float piTimes2 = pi * 2.0;
const vec4 zero4 = vec4( 0.0 );

out vec4 fragment;

void main( void )
{
	// u: horizontal axis
	// v: vertical axis
	// w: origin
	vec2 u, v, w;
	
	//
	// ( fragMax.y < fragMin.y ) is assumed
	{
		w = vec2( fragMin.x, fragMax.y );
		u = fragMax - w;
		v = fragMin - w;
	}

	vec3 prevRayDir = vec3( 0.0 );
	
	vec3 planeNormal = fragTargetPlane.xyz;

	float distanceNumerator = fragTargetPlane.w - dot( frag_Position, planeNormal );
	vec2 invUV = vec2(1.0 / u.x, 1.0 / v.y);

	vec3 normal = normalize( frag_Normal );
	vec4 prevColor = zero4;
	vec4 color = vec4( 0.0 );
	for ( float phi = 0.0; phi <= 3.14159; phi += piOver8 )
	{
		for ( float theta = 0.0; theta <= piTimes2; theta += piOver8 )
		{
			vec3 rayDir = normalize( vec3( cos( theta ) * cos( phi ), sin( phi ), cos( phi ) * sin( theta ) ) );
			float cosAngRay = dot( rayDir, planeNormal );

			if ( cosAngRay >= 0.0 )
			{
				continue;
			}
			
			// Compute distance from origin to sample point
			float t = distanceNumerator / cosAngRay; 

			// Otherwise, continue the use of the integration function: http://www.cs.virginia.edu/~jdl/bib/appearance/analytic%20models/schlick94b.pdf
			vec2 worldSample = (frag_Position + rayDir * t).xz;
			worldSample.x = clamp(worldSample.x, fragMin.x, fragMax.x);
			worldSample.y = clamp(worldSample.y, fragMax.y, fragMin.y);

			vec2 uvSample = clamp((worldSample - w) * invUV, vec2(0.0), vec2(1.0));

			vec4 texel = texture( fragRadianceSampler, uvSample );

			if ( texel != zero4 )
			{
				color += texel * dot( normal, rayDir );
			}
		}
	}

	fragment = vec4( pow( color.rgb, vec3( gamma ) ), color.a );
}