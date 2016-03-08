**3/6/16**

For the glTextures array that's owned by the BSPRenderer class, I'm not sure if that actually needs to be a field.
Either way, one possible method for maintaining consistency with the texture indices (that aren't already used by effect shaders)
while still eliminating redundancy and lowering memory usage for both GPU and CPU, might be using a byte buffer which is "blanked"
out with individual bytes in areas where texture indices aren't actually relevant, and in areas where they *are* relevant, storing
a serialized struct at the appropriate offset. Given the nature of the renderer, any indexing into this data structure would implicitly
guarantee a proper lookup such that no "undefined" or "blanked" out memory was accessed. If the std::unordered_map implementation is at risk
of allocating more memory than desired, even if only a few items are stored, this really could be desirable.

The end result would be storing the texture offsets in the atlas as serializable memory, which pretty much implies ditching
the glm vector usage in this particular case.

Dummy textures which are made shouldn't even be allocated on the GPU; there should be a means of finding them when the atlas
is being constructed, ignoring them, and therefore not even adding their data to the atlas itself. This obviously prevents
unnecessary glTexImage2D calls. If anything, only one dummy texture should be allocated, and any indices which require a dummy should be
marked in the atlas data so that a bind to a dummy can simply be mapped when an arbitrary index is passed.

**3/7/16**

#### Textures

Apparently using GL_BGRA as an input format for textures can prevent having to do intermediate software memory transfers on the GPU...

#### Effect Shaders

Definitely add the following function to `shaderStage_t`:

`const Program& GetProgram( void ) const { return *GQueryProgram( program ); }`

This will help in replacing all of the calls in the renderer.

#### Performance

Running the renderer using Intel drivers gives me 30 FPS at best and over 160 MB of RAM per frame.

On Linux I get 60 MB of ram and 60 FPS with a) intel drivers and b) less convenient features.

I'm thinking the issue might be due to dependencies which are linked in via MSVC, and possibly other things like meta data
for debugging which is applied in VS at runtime.


**On programs ( this should be done AFTER removal of duplicate programs )**

One thing which really needs to happen is that there should be strictly one single buffer for the lazy uniform upload
that happens on program binding for the `Program` class. Each element in that buffer needs to be a union, which maps
to the different primitive types (int, mat4, vec2, etc etc) using strictly primitive data and nothing else (i.e., no GLM).

It *should* be possible to just use the uniform locations as indices into the buffer for accessing the respective elements,
that is assuming that a group of uniforms will have contiguous indices (assuming no -1's exist).

Either way, the buffer should just be allocated once - this will prevent the need for it to be "cleared" each time
the program is released. To avoid always writing every frame, you can design the struct like so:

```
enum uniformType_t
{
	UNIFORM_INT = 0,
	...
	more stuff
	...
	UNIFORM_VEC2,
	UNIFORM_VEC3,
	UNIFORM_VEC4,
	...
	UNIFORM_MAT4
};

struct uniform_t
{
	union data {
		int i;

		...
		more stuff
		...

		float vec2[ 2 ];
		float vec3[ 3 ];
		float vec4[ 4 ];
		...
		float mat4[ 16 ];
	};
	uniformType_t type;
	bool needs_write;
};
```

and then, in the release call, just memset the entire buffer to 0; this will indirectly set needs_write to false.
Keep in mind that this may or may not be faster than just iterating over each element and settings needs_write to false
for each one: you won't really know until you profile and compare.

This is totally a C approach. It's also a much simpler approach than what would be written in idiomatic C++,
and also far more reliable (in this case).

