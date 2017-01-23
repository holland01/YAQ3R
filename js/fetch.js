var AL = {};

AL.bundleLoadPort = '6931';

AL.DATA_DIR_NAME = 'working';
AL.BUNDLE_REQUIRED_PARAMS_EXCEPT = 'Missing path, path length, ' +
	'and/or on load finish callback';
AL.CONTENT_PIPELINE_MSG = false;

AL.fetchNode = function(pathName) {
	var node = null;
	try  {
		node = FS.lookupNode(FS.root, pathName);
	} catch (e) {
		if (e.code === 'ENOENT') {
			FS.mkdir(FS.root.name + pathName);
			node = FS.lookupNode(FS.root, pathName);
		} else {
			throw e;
		}
	}
	return node;
}

AL.mountPackages = function(packages) {
	var node = AL.fetchNode(AL.DATA_DIR_NAME);
	if (!FS.isMountpoint(node)) {
		FS.mount(WORKERFS, {packages: packages},
				'/' + AL.DATA_DIR_NAME);

		if (AL.CONTENT_PIPELINE_MSG) {
			console.log("Mount succeeded");
		}
	} else {
		if (AL.CONTENT_PIPELINE_MSG) {
			console.log("Mount failed");
		} else {
			throw "Mount failed for " + JSON.stringify(packages);
		}
	}
}

AL.unmountPackages = function() {
	var node = AL.fetchNode(AL.DATA_DIR_NAME);
	if (FS.isMountpoint(node)) {
		FS.unmount('/' + AL.DATA_DIR_NAME);
	}
}

AL.getMaybeCString = function(str) {
	var ret = str;
	if (typeof(str) === typeof(0)) {
		ret = UTF8ToString(str);
	} else if (typeof(str) !== 'string') {
		throw 'Invalid string received';
	}
	return ret;
}

AL.setBundleLoadPort = function(port) {
	AL.bundleLoadPort = AL.getMaybeCString(port);
}

AL.writeBufferWord = function(buffer, offset, word) {
	if (buffer) {
		HEAP32[(buffer + offset) >> 2] = word;
	}
}

AL.readBufferWord = function(buffer, offset) {
	if (buffer) {
		return HEAP32[(buffer + offset) >> 2];
	}
	return 0;
}

// If we choose to "map" directly into memory then we just use a
// custom binary format: everything becomes a simple memory access
//---------------
// Format for metadata
//--------------
// [0] 4 byte int -> start (start offset for the segment in the blob)
// [1] 4 byte int -> end (end offset for the segment in the blob)
// [2] string [64] -> filepath (path of the file; used when a pathname is passed)

// offsets for the metadata struct blocks
AL.MD_ELEM_START_OFFSET = 0;
AL.MD_ELEM_END_OFFSET = 4;
AL.MD_ELEM_PATH_OFFSET = 8;
AL.MD_ELEM_PATH_MAX_SIZE = 64;

// size of one metadata struct block
AL.MD_ELEM_SIZE_BYTES = 4 * 2 + AL.MD_ELEM_PATH_MAX_SIZE;

AL.binifyMetadata = function(buffer, metadata) {
	for (let file = 0; file < metadata.files.length; file++) {
		if (metadata.files[file].filename.length >= AL.MD_ELEM_PATH_MAX_SIZE) {
			throw 'File found with excessive large size: '
				+ metadata.files[file].filename +
			'; size: ' + metadata.files[file].filename.length;
		}

		let elem = buffer + file * AL.MD_ELEM_SIZE_BYTES;

		AL.writeBufferWord(elem, AL.MD_ELEM_START_OFFSET, metadata.files[file].start);
		AL.writeBufferWord(elem, AL.MD_ELEM_END_OFFSET, metadata.files[file].end);

		// zero out the string
		Module._memset(elem + AL.MD_ELEM_PATH_OFFSET, 0, AL.MD_ELEM_PATH_MAX_SIZE);

		// passing 'true' ensures that a null term isn't added
		Module.writeStringToMemory(
			metadata.files[file].filename,
			elem + AL.MD_ELEM_PATH_OFFSET,
			true
		);
	}
}

AL.callLoaderCB = function(loader, dataPtr, len)
{
	let stack = Runtime.stackSave();
	Runtime.dynCall('vii',
			loader.params.proxy,
			[dataPtr, len]);
	Runtime.stackRestore(stack);
}

AL.CONTENT_PIPELINE_BENCHMARK = true;

AL.startTime = function() {
	let msb = null;
	if (AL.CONTENT_PIPELINE_BENCHMARK) {
		msb = Date.now();
	}
	return msb;
}

AL.finishTime = function(msb, loader, metaByteLen, blobByteLen) {
	if (AL.CONTENT_PIPELINE_BENCHMARK && msb) {
		console.log(
			"Load Time: ",
			Date.now() - msb,
			"; ",
			loader.bundle, "; ",
			loader.params.path, "mdbin size: ",
			metaByteLen,
			"; Blob size: ",
			blobByteLen
		);
	}
}

// -------------
// Format for the entire buffer
// -------------
// We ultimately pass a buffer which consists of a) the metadata as described above and b) a slice of the blob 
// the metadata corresponds to (this might be the entire blob, or just a slice).
// If it's a slice, then memory offsets need to be taken into account,
// since the metadata is used to index into the blob memory.

// There are some extra 4 byte parameters that are also prefixed to the stream:
// [0] 4 byte int -> offset of the blob slice
// [1] 4 byte int -> size of the blob slice
// [2] 4 byte int -> size of the metadata
// [3] 4 byte int -> the total blob size: this is used to allow the web worker
// 		to determine whether or not they need to query the data a second time.

// metadata for the transfer buffer
AL.BUFFER_PARAM_SLICE_OFFSET = 0;
AL.BUFFER_PARAM_SLICE_SIZE = 4; 
AL.BUFFER_PARAM_MD_LEN = 8;
AL.BUFFER_PARAM_TOTAL_BLOB_SIZE = 12;

AL.BUFFER_PARAM_SIZE_BYTES = 4 * 4;

AL.freeBufferStore = function() {
	if (AL.buffer.ptr) {
		Module._free(AL.buffer.ptr);
		AL.buffer.ptr = 0;
	}
}

AL.buffer = {
	ptr: 0,
	size: 0,
	blobOffset: 0,
	metaByteLen: 0,
	blobByteLen: 0
};

// Allocate a slice of the blob 
// and then store it within the heap.
// Ideally, we can store the entire blob...
AL.allocSlice = function(loader, start, end) {	
	let fileReader = new FileReaderSync();
	let blobBuff = null;

	if (start && end) { 
		blobBuff = fileReader.readAsArrayBuffer(loader.packageRef.blob.slice(start, end));
	} else {
		blobBuff = fileReader.readAsArrayBuffer(loader.packageRef.blob);
	}

	AL.buffer.blobOffset = start || 0;
	AL.buffer.metaByteLen = loader.packageRef.metadata.files.length * AL.MD_ELEM_SIZE_BYTES;
	AL.buffer.blobByteLen = blobBuff.byteLength;

	// See if we need to grab some memory.
	// Even if AL.buffer.ptr is null this will work fine
	let alBlobSize = AL.readBufferWord(AL.buffer.ptr, AL.BUFFER_PARAM_SLICE_SIZE);
	
	if (alBlobSize !== AL.buffer.blobByteLen) {
		AL.buffer.size = AL.BUFFER_PARAM_SIZE_BYTES + AL.buffer.metaByteLen + AL.buffer.blobByteLen;
		if (AL.buffer.blobByteLen > alBlobSize) {
			AL.freeBufferStore();
			AL.buffer.ptr = Module._malloc(AL.buffer.size);
		}
	}

	// Write out prefix data
	AL.writeBufferWord(AL.buffer.ptr, AL.BUFFER_PARAM_SLICE_OFFSET, AL.buffer.blobOffset);
	AL.writeBufferWord(AL.buffer.ptr, AL.BUFFER_PARAM_SLICE_SIZE, AL.buffer.blobByteLen);
	AL.writeBufferWord(AL.buffer.ptr, AL.BUFFER_PARAM_MD_LEN, AL.buffer.metaByteLen);
	AL.writeBufferWord(AL.buffer.ptr, AL.BUFFER_PARAM_TOTAL_BLOB_SIZE, 
		loader.packageRef.blob.size);

	// Write out blob metadata
	AL.binifyMetadata(AL.buffer.ptr + AL.BUFFER_PARAM_SIZE_BYTES, 
		loader.packageRef.metadata);

	// Write out the blob slice
	let blobBytes = new Uint8Array(blobBuff);
	Module.writeArrayToMemory(
		blobBytes,
		AL.buffer.ptr + AL.BUFFER_PARAM_SIZE_BYTES + AL.buffer.metaByteLen
	);
}

AL.loaderCache = null;

AL.loadFinished = function(loader) {
	if (loader.params.map) {
		let msb = AL.startTime();		

		AL.allocSlice(loader);
		AL.callLoaderCB(loader, AL.buffer.ptr, AL.buffer.size);
		
		AL.finishTime(msb, loader, AL.buffer.metaByteLen, AL.buffer.blobByteLen);
	} else {
		AL.mountPackages([loader.packageRef]);
		
		let pbuf = 0;
		let pbufLen = 0;

		if (loader.params.path) {
			pbuf = Module._malloc(loader.params.size + 1);

			// NOTE: writeStringToMemory creates an additional array,
			// which is where the null term will be added if at all;
			// every element is then copied into pbuf.
			// So, if there is a null term (which there almost certainly is),
			// then pbuf's extra byte will suffice.
			Module.writeStringToMemory(loader.params.path, pbuf, false);
			pbufLen = loader.params.size;
		}

		AL.callLoaderCB(loader, pbuf, pbufLen);

		if (pbuf) {
			Module._free(pbuf);
		}
	}
}

AL.BundleLoader = function(bundle, params) {
	this.bundle = bundle;
	this.packageRef = {metadata:null, blob:null};
	this.fin = {metadata:false, blob:false};

	if (!params
		|| (typeof(params.size) === 'undefined')
		|| (typeof(params.path) === 'undefined')
		|| !params.proxy) {
		throw AL.BUNDLE_REQUIRED_PARAMS_EXCEPT;
	}

	if (params.port) {
		AL.setBundleLoadPort(params.port);
	}

	this.params = params;
}

AL.BundleLoader.prototype.load = function() {
	this.xhrRequest('blob', 'blob', '.data');
	this.xhrRequest('json', 'metadata', '.js.metadata');
}

AL.BundleLoader.prototype.xhrRequest = function(responseType,  packRefKey, ext) {
	var xhr = new XMLHttpRequest();
	var url = 'http://localhost:' + AL.bundleLoadPort
		+ '/bundle/' + this.bundle + ext;

	if (AL.CONTENT_PIPELINE_MSG) {
		console.log('URL Constructed: ', url);
	}

	xhr.open('GET', url);
	xhr.responseType = responseType;

	xhr.setRequestHeader('Access-Control-Allow-Origin',
			'http://localhost:' + AL.bundleLoadPort);

	xhr.addEventListener('readystatechange', function(evt) {
		if (AL.CONTENT_PIPELINE_MSG) {
			console.log('XHR Ready State: '
					+ xhr.readyState
					+ 'XHR Status: ' + xhr.status);
		}

		if (xhr.readyState === XMLHttpRequest.DONE) {
			if (AL.CONTENT_PIPELINE_MSG) {
				console.log('DONE for ', url);
			}
			this.packageRef[packRefKey] = xhr.response;
			this.fin[packRefKey] = true;

			if (this.fin.metadata && this.fin.blob) {
				AL.loadFinished(this);
			}
		}
	}.bind(this));

	xhr.send();
}

AL.fetchBundleAsync = function(bundleName, callback, path, pathLength, port, map) {
	var loader = new AL.BundleLoader(
		AL.getMaybeCString(bundleName), {
			proxy: callback,
			path: AL.getMaybeCString(path),
			size: pathLength,
			port: port,
			CONTENT_PIPELINE_MSG: false,
			map: map || false
		}
	);

	loader.load();
}

self.fetchBundleAsync = AL.fetchBundleAsync;
self.unmountPackages = AL.unmountPackages;
self.mountPackages = AL.mountPackages;

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
				var u8buf = intArrayFromString(p, true);

				var pbuf = Module._malloc(u8buf.length);
				Module.writeArrayToMemory(u8buf, pbuf);

				callfn(pbuf, u8buf.length);
				Module._free(pbuf);
			 }
		 }
		 return false;
	 }

	 traverse(root);

	 // Signal the end
	 callfn(0, 0);

	 return 1;
}

self.walkFileDirectory = walkFileDirectory;
