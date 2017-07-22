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
