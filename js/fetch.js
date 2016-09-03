function BundleLoader() {
		
}


function xhrSnagFromName(exts, key, name, events, param,
	packages, bundlePairIndex, threshold, portNumber, eventIndex) {

	// 6931 is the default port provided by emscripten
	var portStr = (!!portNumber ? portNumber.toString() : '6931');

	var xhr = new XMLHttpRequest();
	var url = 'http://localhost:' + portStr + '/bundle/' + name + key;
	console.log('Loading ', url, '...');
	xhr.open('GET', url);
	xhr.responseType = exts[key]['type'];
	xhr.setRequestHeader('Access-Control-Allow-Origin',
		'http://localhost:' + portStr);
	xhr.addEventListener('readystatechange', function(evt) {
		console.log('XHR Ready State: ' + xhr.readyState
			+ 'XHR Status: ' + xhr.status);
		if (xhr.readyState === XMLHttpRequest.DONE) {
			console.log('status 200; writing response: ', xhr.response);
			param[exts[key]['param']] = xhr.response;
			if (eventIndex < events.length) {
				f = events[eventIndex];
				f(events, exts, name, packages, param, bundlePairIndex,
					threshold, eventIndex);
			}
		}
	});
	xhr.send();
}

var funcEvents = [
	function(events, exts, name, packages, param, bpi, threshold, eventIndex){
		xhrSnagFromName(exts, '.data', name, events, param,
			packages, bpi, threshold, port, eventIndex + 1);
	},
	function(events, exts, name, packages, param, bpi, threshold, eventIndex){
		xhrSnagFromName(exts, '.js.metadata', name, events, param,
			packages, bpi, threshold, port, eventIndex + 1);
	},
	function(events, exts, name, packages, param, bpi, threshold, eventIndex) {
		packages.push(param);
		if (bpi !== threshold) {
			return;	
		}

		console.log('the last is hit');

	}
];

function fetchBundleAsync(names, finished, exts, packages, threshold, port) {
	console.log('Loading bundles with the following names: ', JSON.stringify(names));
	var params = new Array(names.length);
	for (var i = 0; i < names.length; ++i) {
		params[i] = {metadata:null, blob: null};
	}
	
/*
	for (var i = 0; i < names.length; ++i) {	
		xhrSnagFromName(exts, 
				'.data', 
				names[i], 
				funcEvents, 
				params[i],
				packages, i, 
				names.length - 1,
				port, 
				0				// event index
		);
	}
*/
}
function beginFetch(proxy, path, size, strPortNum) {	
	var bundles = [
		['sprites', 'gfx'],
		['scripts', 'maps'],
		['textures', 'env'], 
		['models']
	];
	var fetchCount = 0;
	function onFinish(packagesRef) {
		fetchCount++;
		console.log('fetch count: ', fetchCount);
		if (fetchCount === bundles.length) {
			console.log("HEAP BEFORE: ", DYNAMICTOP);
			
			
			FS.mkdir('/working');
			FS.mount(WORKERFS, {
				packages: packagesRef },
				'/working');

			console.log("HEAP AFTER: ", DYNAMICTOP);


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

	if (strPortNum) {
		if (typeof(strPortNum) === typeof(0)) {
			strPortNum = UTF8ToString(strPortNum);
		} else if (typeof(strPortNum) !== 'string') {
			throw 'invalid value received for the server port';
		}
	} else {
		strPortNum = null;
	}

	for (var i = 0; i < bundles.length; ++i) {
		fetchBundleAsync(bundles[i], onFinish, exts, packages,
			bundles.length - 1, strPortNum);
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
