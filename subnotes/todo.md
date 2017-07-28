### 5/4/2017

* ~~make sure that the optimized build doesn't crash~~

~~* Try the frustum idea and see if it helps with the framerate when the camera flies out in the open. Make sure you compare FPS before and after; maybe use a flag that computes
the extra paralell planes if true and doesn't if false.~~

* Forget about the Frustum for now, see about leveraging timing via input and desired
framerate using the camera update method and tracking the desired FPS.

### 5/13/2017

* Figure out source of emscripten compilation issue: you need makeThrowVerbose to work so you can diagnose what's causing
the renderer to freeze when q3dm13.bsp is loaded.

### 5/14/2017

* Despite modifications to parseTools.js and library.js the thrown exception doesn't seem to take into account the extra arguments passed. Figure out
why...

### 5/20/2017

crash happens in DrawMapPass, with a non-zero texture index
called into BindTexture. Here is the offending call:
(the heap16 buffer is minified, represented as 'b').



__ZN11BSPRenderer11BindTextureERK7ProgramRKNSt3__210unique_ptrIN3gla7atlas_tENS3_14default_deleteIS6_EEEEtPKci(0, j, i, b[(__ZNKSt3__213unordered_mapIjtNS_4hashIjEENS_8equal_toIjEENS_9allocatorINS_4pairIKjtEEEEE2atERS7_(m + 128 | 0, h) | 0) >> 1] | 0, 29543, 0)

entire minified function:

(note that 'a' variable is the this pointer)

```
function __ZN11BSPRenderer11DrawMapPassEiiNSt3__28functionIFvRK7ProgramEEE(a, d, e, f) {
       a = a | 0;
       d = d | 0;
       e = e | 0;
       f = f | 0;
       var g = 0
         , h = 0
         , i = 0
         , j = 0
         , k = 0
         , m = 0;
       g = l;
       l = l + 16 | 0;
       h = g;
       Xf(515);
       Vd(1, 0);
       c[h >> 2] = 0;
       c[h + 4 >> 2] = 0;
       c[h + 8 >> 2] = 0;
       __ZNSt3__212basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEE6__initEPKcj(h, 29410, 4);
       i = __ZNSt3__212__hash_tableINS_17__hash_value_typeINS_12basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEEENS_10unique_ptrI7ProgramNS_14default_deleteIS9_EEEEEENS_22__unordered_map_hasherIS7_SD_NS_4hashIS7_EELb1EEENS_21__unordered_map_equalIS7_SD_NS_8equal_toIS7_EELb1EEENS5_ISD_EEE4findIS7_EENS_15__hash_iteratorIPNS_11__hash_nodeISD_PvEEEERKT_(a + 36 | 0, h) | 0;
       if (!i) {
           j = Yf(8) | 0;
           __ZNSt11logic_errorC2EPKc(j, 39974);
           c[j >> 2] = 27356;
           _b(j | 0, 2560, 392)
       }
       j = c[i + 20 >> 2] | 0;
       __ZNSt3__212basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEED2Ev(h);
       __ZNK7Program25LoadDefaultAttribProfilesEv(j);
       if ((d | 0) < 0) {
           i = a + 8 | 0;
           __ZN11BSPRenderer11BindTextureERK7ProgramRKNSt3__210unique_ptrIN3gla7atlas_tENS3_14default_deleteIS6_EEEEtPKci(0, j, i, (c[c[i >> 2] >> 2] | 0) + 65535 & 65535, 29543, 0);
           k = i
       } else {
           i = a + 4 | 0;
           m = c[i >> 2] | 0;
           c[h >> 2] = d;
           __ZN11BSPRenderer11BindTextureERK7ProgramRKNSt3__210unique_ptrIN3gla7atlas_tENS3_14default_deleteIS6_EEEEtPKci(0, j, i, b[(__ZNKSt3__213unordered_mapIjtNS_4hashIjEENS_8equal_toIjEENS_9allocatorINS_4pairIKjtEEEEE2atERS7_(m + 128 | 0, h) | 0) >> 1] | 0, 29543, 0);
           k = a + 8 | 0
       }
       if ((e | 0) < 0)
           __ZN11BSPRenderer11BindTextureERK7ProgramRKNSt3__210unique_ptrIN3gla7atlas_tENS3_14default_deleteIS6_EEEEtPKci(0, j, k, (c[c[a + 8 >> 2] >> 2] | 0) + 65535 & 65535, 39960, 1);
       else
           __ZN11BSPRenderer11BindTextureERK7ProgramRKNSt3__210unique_ptrIN3gla7atlas_tENS3_14default_deleteIS6_EEEEtPKci(0, j, k, e & 65535, 39960, 1);
       c[h >> 2] = 0;
       c[h + 4 >> 2] = 0;
       c[h + 8 >> 2] = 0;
       __ZNSt3__212basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEE6__initEPKcj(h, 35540, 11);
       __ZNK7Program8LoadMat4ERKNSt3__212basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEERKN3glm7tmat4x4IfLNS9_9precisionE0EEE(j, h, (c[a + 120 >> 2] | 0) + 72 | 0);
       __ZNSt3__212basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEED2Ev(h);
       __ZNK7Program4BindEv(j);
       h = c[f + 16 >> 2] | 0;
       if (!h) {
           f = Yf(4) | 0;
           c[f >> 2] = 3308;
           _b(f | 0, 168, 48)
       } else {
           kh[c[(c[h >> 2] | 0) + 24 >> 2] & 255](h, j);
           __ZNK7Program7ReleaseEv(j);
           Pa(33984);
           ad(3553, 0);
           Pa(33985);
           ad(3553, 0);
           l = g;
           return
       }
   }
```
### 7/21/2017

It's been 2 months since I've touched this; thankfully I'm pretty sure I've figured
out the issue. Here's the thing: invalid image paths might not be inserted
into the key map. That's fine. BUT, that doesn't change the fact that
bspFace_t entries are going to have indices into the bspShader_t buffer
and in turn will be used to access image info from an external atlas key map.

So, with two images removed there's an out of bounds access which occurs
and causes the crash. The index is 99, so apparently that would be
the flareShader entry.

What's left to do is finish up the Mark/Sweep deal so that once the map load has finished
the list of bad indices can be iterated over so the map data is modified as needed.

### 7/23/2017

There are going to be shaders without valid filepaths; these are still important to
keep, though, because the bspFace_t list will likely contain faces which refer
to them via the shader/texture index being lsited. For some reason one shader
in particular is elusive in the sense that it's never even detected by the
async_io image loader...WTF???

### 7/24/2017

There's some helper functions in debug_util.js which will help you get
the values of BSPRenderer::map and Q3BspMap::name. What you want is to write
another helper function which will give you the name of a bspShader_t entry
given an index. In this case, you want to look up entry 68, as the name
for that shader should provide some insight as to what is going on...

### 7/25/2017


**First Bit**


IMPORTANT: binary layout for basic_string is the definition used when
_LIBCPP_ABI_ALTERNATE_STRING_LAYOUT IS defined

(at least on Linux - this may or may not vary between platforms)

```
struct __long
{
    pointer   __data_;
    size_type __size_;
    size_type __cap_;
};

struct __short
{
    value_type __data_[__min_cap];
    struct
        : __padding<value_type>
    {
        unsigned char __size_;
    };
};

struct __raw
{
    size_type __words[__n_words];
};

struct __rep
{
    union
    {
        __long  __l;
        __short __s;
        __raw   __r;
    };
};
```

"textures/gothic_door/tim_dmarch01"

**Second Bit**

The data for q3dm13 is large enough to where the SEGMENT_SIZE variable
in fetch.js:AL.addSliceMeta is less than the blob's data. When this was
originally written, an outline was created which handled the data accordingly,
but didn't finish the necessary lifting required for fetching remaining slices
necessary.

You'll need to modify this fetch.js code (and probably some code in other
areas, including file_traverse.cxx) so that it continuously loads the next
slice as needed. This will involve a back-and-forth like mechanism
between LoadAndStreamImages() and AL.loadFinished - the code in async_image_io
shouldn't have to worry about this at all, unless the iterator information
it has is inconsistent, which is possible (compare the termination mechanism
being used).

Starting on line 358 in fetch.js is this:

```
// Here we're done: we only care about requested files that exist in
// the bundle at this point. If we find that enough images
// supersede our memory constraints then the implementation
// will be altered.
metadata.files = null;
```

You want to comment out this line, given that the comment above insinuates that
this was pretty much during the time when slices weren't necessary. They
are now, though.

You could also ditch this entire content pipeline deal and throw together
a tool which packages all data necessary for one single map into a binary
file. This would eliminate load time issues altogether, because a smaller
image can just be preloaded into the binary as was the case originally.

### 7/26/17

1) Make sure slice offsets are kept track of.

NOTE: there's a crash which happens in the compiled version of LoadImagesBegin.
Looks like there's an issue involving the gPathMap_t vector. It could
be inlining AIIO_ReadImages, and there's areas where the vector is
passed to a constructor and hence possibly moved (it's not a reference pass).

If so, there's code right after which uses the now-empty vector variable and
that could be causing issues. It's been replaced with the image tracker's buffer,
so just test it and make sure it works.

One last thing: the issue seems to be similar to what was happening before,
so move semantics may also be playing a role here, outside of just the
amount of data obtained.

### 7/28/17

Somehow a bad bundle path is getting passed via GU_LoadShaderTextures

bspviewer.html:1237
bspviewer.html:1237 [ GU_LoadShaderTextures | INFO ]: Called
file_traverse.js:171 ALL = |
file_traverse.js:171 BUNDLE = ; PATHS =
VM26:72 TYPEOF STR =  number ; STRING VALUE =  5267280 ; ret =  
VM26:72 TYPEOF STR =  number ; STRING VALUE =  5267281 ; ret =  
VM26:72 TYPEOF STR =  number ; STRING VALUE =  10288 ; ret =  6931
worker/file_traverse.js:171 ALL = |
worker/file_traverse.js:171 BUNDLE = ; PATHS =
3(unknown) TYPEOF STR =
bundle/.data Failed to load resource: the server responded with a status of 404 (File not found: /home/amsterdam/Devel/proj/yaq3r/bundle/.data)
bundle/.js.metadata Failed to load resource: the server responded with a status of 404 (File not found: /home/amsterdam/Devel/proj/yaq3r/bundle/.js.metadata)
VM26:430 THE PATH =  
(unknown) THE PATH =
VM26:329 Uncaught Expected a pipe delimited list of file paths for the bundle
worker/file_traverse.js:329 Uncaught Expected a pipe delimited list of file paths for the bundle


NOTES
====

* There shouldn't be any issues with sending the files provisionally: it looks
like the event queue works itself out here and prevents race conditions, so
things remain synchronous.

* Don't forget that if one image file isn't found for a given bundle
that bundle will get sacked; consider just sending provisional NULL data
so the client can adjust the data for the renderer accordingly

* The "paths" which are passed to the loader aren't being used; you don't
want to load the entire blob, but only mere slices of the blob corresponding to the
files you wanted to send. That's why the setup was working in the manner it was.
The fetch.js file is reverted so this should be taken care of now.
