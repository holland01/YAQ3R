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

Apparently using GL_BGRA as an input format for textures can prevent having to do intermediate software memory transfers on the GPU...


