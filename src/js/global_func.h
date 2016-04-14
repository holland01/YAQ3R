#pragma once

#ifdef EMSCRIPTEN

#define EM_JS_SCRIPT(script) #script"\n"

// Async execution to snag some data; is designed to
// be ran in parallel with other requests
#define EM_FUNC_XHR_SNAG_FROM_NAME \
EM_JS_SCRIPT(\
	function xhrSnagFromName(exts, key, name, next, param,\
		packages) {\
		var xhr = new XMLHttpRequest();\
		var url = 'http://localhost:6931/bundle/' + name + key;\
		console.log('Loading ', url, '...');\
		xhr.open('GET', url);\
		xhr.responseType = exts[key]['type'];\
		xhr.setRequestHeader('Access-Control-Allow-Origin',\
			'http://localhost:6931');\
		xhr.addEventListener('readystatechange', function(evt) {\
			console.log('XHR Ready State: ' + xhr.readyState\
				+ '\nXHR Status: ' + xhr.status);\
			if (xhr.readyState === XMLHttpRequest.DONE) {\
				console.log('status 200; writing...');\
				param[exts[key]['param']] = xhr.response;\
				if (next.length > 0) {\
					f = next.shift();\
					f(next, exts, name, packages);\
				}\
			}\
		});\
		xhr.send();\
	}\
)


// param will be written to once each xhr request
// is finished; in addition, at the end of each request,
// the first function in funcEvents is 'popped'
// and then executed to signal a push of the param.
// Eventually, we hit the last set of names,
// and then call our finished return point which
// updates its own data.
#define EM_FUNC_FETCH_BUNDLE_ASYNC \
EM_JS_SCRIPT(\
	function fetchBundleAsync(names, finished, exts, packages) { \
		console.log('Loading bundles with the following names: ', JSON.stringify(names)); \
		for (var i = 0; i < names.length; ++i) { \
			var param = {}; \
			var funcEvents = [ \
				function(next, exts, name, packcages) { \
					xhrSnagFromName(exts, '.js.metadata', names[i], param, next, packages); \
				}, \
				function(next, exts, name, packages) { \
					packages.push(param); \
					if (i === names.length - 1) { \
						finished(packages); \
					} \
				} \
			]; \
			xhrSnagFromName(exts, '.data', names[i], param, funcEvents, packages); \
		} \
	})

#define EM_FUNC_WALK_FILE_DIRECTORY \
EM_JS_SCRIPT(\
	(function walkFileDirectory($0, $1, $2) { \
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
	} \
	function xhrSnagFromName(exts, key, name, next, param,\
		packages) {\
		var xhr = new XMLHttpRequest();\
		var url = 'http://localhost:6931/bundle/' + name + key;\
		console.log('Loading ', url, '...');\
		xhr.open('GET', url);\
		xhr.responseType = exts[key]['type'];\
		xhr.setRequestHeader('Access-Control-Allow-Origin',\
			'http://localhost:6931');\
		xhr.addEventListener('readystatechange', function(evt) {\
			console.log('XHR Ready State: ' + xhr.readyState\
				+ '\nXHR Status: ' + xhr.status);\
			if (xhr.readyState === XMLHttpRequest.DONE) {\
				console.log('status 200; writing...');\
				param[exts[key]['param']] = xhr.response;\
				if (next.length > 0) { \
					f = next.shift(); \
					f(next, exts, name, packages); \
				} \
			} \
		}); \
		xhr.send(); \
	} \
	function fetchBundleAsync(names, finished, exts, packages) { \
		console.log('Loading bundles with the following names: ', JSON.stringify(names)); \
		for (var i = 0; i < names.length; ++i) { \
			var param = {}; \
			var funcEvents = [ \
				function(next, exts, name, packcages) { \
					xhrSnagFromName(exts, '.js.metadata', names[i], param, next, packages); \
				}, \
				function(next, exts, name, packages) { \
					packages.push(param); \
					if (i === names.length - 1) { \
						finished(packages); \
					} \
				} \
			]; \
			xhrSnagFromName(exts, '.data', names[i], param, funcEvents, packages); \
		} \
	}) \
)

#endif // EMSCRIPTEN
