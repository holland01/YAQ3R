function xhrSnagFromName(exts, key, name, next, param,
	packages, bundlePairIndex, threshold) {
	var xhr = new XMLHttpRequest();
	var url = 'http://localhost:6931/bundle/' + name + key;
	console.log('Loading ', url, '...');
	xhr.open('GET', url);
	xhr.responseType = exts[key]['type'];
	xhr.setRequestHeader('Access-Control-Allow-Origin',
		'http://localhost:6931');
	xhr.addEventListener('readystatechange', function(evt) {
		console.log('XHR Ready State: ' + xhr.readyState
			+ 'XHR Status: ' + xhr.status);
		if (xhr.readyState === XMLHttpRequest.DONE) {
			console.log('status 200; writing response: ', xhr.response);
			param[exts[key]['param']] = xhr.response;
			if (next.length > 0) {
				f = next.shift();
				f(next, exts, name, packages, param, bundlePairIndex,
					threshold);
			}
		}
	});
	xhr.send();
}
function fetchBundleAsync(names, finished, exts, packages, threshold) {
	console.log('Loading bundles with the following names: ', JSON.stringify(names));
	var params = new Array(names.length);
	for (var i = 0; i < names.length; ++i) {
		params[i] = {metadata:null, blob: null};
	}
	for (var i = 0; i < names.length; ++i) {
		var funcEvents = [
			function(next, exts, name, packages, param, bpi, threshold){
				xhrSnagFromName(exts, '.data', name, next, param,
					packages, bpi, threshold);
			},
			function(next, exts, name, packages, param, bpi, threshold){
				xhrSnagFromName(exts, '.js.metadata', name, next, param,
					packages, bpi, threshold);
			},
			function(next, exts, name, packages, param, bpi, threshold) {
				packages.push(param);
				if (bpi === threshold) {
					console.log('the last is hit');
					finished(packages);
				}
			}
		];
		xhrSnagFromName(exts, '.data', names[i], funcEvents, params[i],
			packages, i, names.length - 1);
	}
}
function beginFetch(proxy, path, size) {
	var bundles = [['sprites', 'gfx'],['scripts', 'maps'],['textures', 'env']];
	var fetchCount = 0;
	function onFinish(packagesRef) {
		fetchCount++;
		console.log('fetch count: ', fetchCount);
		if (fetchCount === bundles.length) {
			FS.mkdir('/working');
			FS.mount(WORKERFS, {
				packages: packagesRef },
				'/working');
			var stack = Runtime.stackSave();
			Runtime.dynCall('vii', proxy, [path, size]);
			Runtime.stackRestore(stack);
		}
	}
	var exts = {
		'.data': {
			'type': 'blob',
			'param': 'blob'
		},
		'.js': {
			'type': 'text',
			'param': '.js'
		},
		'.js.metadata': {
			'type': 'json',
			'param': 'metadata'
		}
	};
	var packages = [];
	for (var i = 0; i < bundles.length; ++i) {
		fetchBundleAsync(bundles[i], onFinish, exts, packages,
			bundles.length - 1);
	}
}
function walkFileDirectory(pathPtr, callbackPtr, errPtr) {
	 var path = UTF8ToString(pathPtr);
	 var lookup = FS.lookupPath(path);
	 if (!lookup) {
		 stringToUTF8('Path given ' + path + ' could not be found.', errPtr, 128);
		 return 0;
	 }
	 var root = lookup.node;
	 var iterate = true;

	 function callfn(data, size) {
		 var stack = Runtime.stackSave();
		 Runtime.dynCall('vii', callbackPtr, [data, size]);
		 Runtime.stackRestore(stack);
	 }

	 function traverse(node) {
		 var path = FS.getPath(node);
		 var stat = FS.stat(path);
		 if (FS.isFile(stat.mode)) {
			 return true;
		 }
		 for (var n in node.contents) {
			 if (!iterate) {
				 break;
			 }
			 if (traverse(node.contents[n])) {
				 var p = FS.getPath(node.contents[n]);
				 var u8buf = intArrayFromString(p);
				 console.log('Iterating path: ', p, ' Size: ', p.length,
			 		' u8 Size: ', u8buf.length);
				 var pbuf = Module._malloc(u8buf.length);
				 Module.writeArrayToMemory(u8buf, pbuf);
				 // very, very important that p.length is used:
				 // u8buf will add an extra null terminator, despite the fact that
				 // p already has one...this may be related to multibyte characteristics,
				 // but for this case it makes sense to just pass the original size
				 callfn(pbuf, p.length);
				 Module._free(pbuf);
			 }
		 }
		 return false;
	 }
	 traverse(root);
	 callfn(0, 0);

	 return 1;
}
self.walkFileDirectory = walkFileDirectory;
self.fetchBundleAsync = fetchBundleAsync;
self.xhrSnagFromName = xhrSnagFromName;
self.beginFetch = beginFetch;
