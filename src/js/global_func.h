#pragma once

#ifdef EMSCRIPTEN

#define EM_JS_SCRIPT(script) #script"\n"

#define EM_FUNC_WALK_FILE_DIRECTORY \
EM_JS_SCRIPT( \
	function walkFileDirectory($0, $1, $2) { \
		 var path = UTF8ToString($0); \
		 var lookup = FS.lookupPath(path); \
		 if (!lookup) { \
			 stringToUTF8('Path given ' + path + ' could not be found.', $2, 128); \
			 return 0; \
		 } \
		 var root = lookup.node; \
		 var iterate = true; \
		 function traverse(node) { \
			 var path = FS.getPath(node); \
			 var stat = FS.stat(path); \
			 if (FS.isFile(stat.mode)) { \
				 return true; \
			 } \
			 for (var n in node.contents) { \
				 if (!iterate) { \
					 break; \
				 } \
				 if (traverse(node.contents[n])) { \
					 var p = FS.getPath(node.contents[n]); \
					 var u8buf = intArrayFromString(p); \
					 var pbuf = Module._malloc(u8buf.length); \
					 Module.writeArrayToMemory(u8buf, pbuf); \
					 var stack = Runtime.stackSave(); \
					 iterate = !!Runtime.dynCall('ii', $1, [pbuf]); \
					 Runtime.stackRestore(stack); \
					 Module._free(pbuf); \
				 } \
			 } \
			 return false; \
		 } \
		 traverse(root); \
		 return 1; \
	})

#endif // EMSCRIPTEN
