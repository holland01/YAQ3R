- Uniform Block:
	- Used by all programs (unless stated otherwise)
	- Holds the following members which are deemed universal:
		- mat4 modelView	// world to view transformation
		- mat4 projection	// view to clip transformation

- if tcMod is specified; a uniform texture transformation is required for the stage
	- Rotate -> glm::rotate
	- Stretch -> glm::scale
	- a glm::mat4 will be generated which can be passed in as a uniform

- for blendFunc, both dest and src values must be > 0 (i.e., blendDest and blendSrc)

- sampler objects should be used in place of tex parameters.

- for alphaGen:
	- all programs will have a constant alphaGen float value.
	- if not speciifed in the stage of an entry, the constant will default to 1.0

- for rgbGen:
 	- vertex attributes:
 		- Each attribute should be used only when necessary.
 		- frag_Position: interpolated position of vertex in the fragment shader 
 			- NOTE: if it's possible for vertex data to be passed in local space,
 					this should be handled. For now, they're are assumed to be in world space (which is the case for map models, at the very least - entities themselves may be different)
 		- frag_Tex: interpolated tex coords in the fragment shader 
 		- frag_Normal: interpolated normal in the fragment shader
 		- frag_Color: yes
 	- uniforms:
 		- for lightvol-based effects:
	 		- vec3 dirToLight: direction to the lightmap for the shader.
	 			- 	the direction to the light can found as an entry in the lightvols lump,
	 				however it is specified in spherical coordinates and henceforth must be converted (this should be done on the CPU prior).

	 		- vec3 ambient: ambient lighting from lightvol. 
	 			- is normalized (i.e., is an array of 3 bytes in RGB format); use GL_TRUE as the normalized flag for specifying the vertex layout
	 			-   
	- Vertex -> 
		fragment = vec4( texture( samplerImage, frag_Tex ).rgb * frag_Color.rgb, alphaGen ) * ambient
	- oneMinusVertex -> 
		fragment = vec4( vec3( 1.0 ) - texture( samplerImage, frag_Tex ).rgb * frag_Color.rgb, alphaGen ) * ambient
	- lightingDiffuse -> (?: does this include specular?)
		float diffuse = dot( dirToLight, frag_Position ); // compute our angle of incidence component
		fragment = vec4( texture( samplerImage, frag_Tex ).rgb * diffuse * frag_Color.rgb, alphaGen ) * ambient

		
