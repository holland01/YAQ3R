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

AL.binifyMetadata = function(buffer, files) {
	for (let file = 0; file < files.length; file++) {
		if (files[file].filename.length >= AL.MD_ELEM_PATH_MAX_SIZE) {
			throw 'File found with excessive large size: '
				+ files[file].filename +
			'; size: ' + files[file].filename.length;
		}

		let elem = buffer + file * AL.MD_ELEM_SIZE_BYTES;

		AL.writeBufferWord(elem, AL.MD_ELEM_START_OFFSET, files[file].start);
		AL.writeBufferWord(elem, AL.MD_ELEM_END_OFFSET, files[file].end);

		// zero out the string
		Module._memset(elem + AL.MD_ELEM_PATH_OFFSET, 0, AL.MD_ELEM_PATH_MAX_SIZE);

		// passing 'true' ensures that a null term isn't added
		Module.writeStringToMemory(
			files[file].filename,
			elem + AL.MD_ELEM_PATH_OFFSET,
			true
		);
	}
}

AL.callLoaderCB = function(loader, dataPtr, len)
{
	let stack = stackSave();
	dynCall('vii',
			loader.params.proxy,
			[dataPtr, len]);
	stackRestore(stack);
}

AL.CONTENT_PIPELINE_BENCHMARK = false;

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

AL.buffer = {
	ptr: 0,
	size: 0,
	blobOffset: 0,
	metaByteLen: 0,
	blobByteLen: 0,
	hasMetaData: false,
	loader: null,
	slicesLeft: 0
};

AL.sliceMeta = [];

AL.texType = {
	SHADER: 0,
	DEFAULT: 1
};

// -------------
// Format for the entire buffer
// -------------
// We ultimately pass a buffer to the loader callback which
// consists of a) the metadata as described above and b) a slice of the blob
// the metadata corresponds to (this might be the entire blob, or just a slice).

// If it's a slice, then memory offsets need to be taken into account,
// since the metadata is used to index into the blob memory.

// These are the 4 byte parameters that are at the beginning of the stream:

// [0] 4 byte int -> offset of the blob slice
// [1] 4 byte int -> size of the blob slice
// [2] 4 byte int -> the number of slices left to be received by the client.
// [3] 4 byte int -> metadata byte length

// metadata for the transfer buffer
AL.BUFFER_PARAM_SLICE_OFFSET = 0;
AL.BUFFER_PARAM_SLICE_SIZE = 4;
AL.BUFFER_PARAM_SLICES_LEFT = 8;
AL.BUFFER_PARAM_MD_LEN = 12;

AL.BUFFER_PARAM_SIZE_BYTES = 4 * 4;

AL.freeBuffer = function() {
	if (AL.buffer.ptr) {
		Module._free(AL.buffer.ptr);
		AL.buffer.ptr = 0;
	}
}

AL.clearBuffer = function() {
	AL.freeBuffer();

	AL.buffer.size = 0;
	AL.buffer.blobOffset = 0;
	AL.buffer.metaByteLen = 0;
	AL.buffer.blobByteLen = 0;
	AL.buffer.hasMetaData = false;
	AL.buffer.loader = null;
	AL.buffer.slicesLeft = 0;

	AL.sliceMeta = [];
}

// Allocate a slice of the blob
// and then store it within the heap.
// Ideally, we can store the entire blob...
AL.allocMem = function(loader, metadata) {
	if (!loader || typeof(start) == undefined || typeof(end) == undefined) {
		throw "Undefined or null param received - this isn't allowed.";
	}

	let fileReader = new FileReaderSync();
	let blobBuff = [];

	AL.buffer.blobByteLen = 0;
	for (let i = 0; i < AL.sliceMeta.length; ++i) {

		// In the event that metadata === AL.sliceMeta,
		// we're taking only what we need from the bundle,
		// so we need to adjust the offsets accordingly for the client.
		let start = AL.buffer.blobByteLen;
		AL.buffer.blobByteLen += AL.sliceMeta[i].end - AL.sliceMeta[i].start;
		let end = AL.buffer.blobByteLen;

		blobBuff.push(
			fileReader.readAsArrayBuffer(
				loader.packageRef.blob.slice(
					AL.sliceMeta[i].start,
					AL.sliceMeta[i].end
				)
			)
		);

		AL.sliceMeta[i].start = start;
		AL.sliceMeta[i].end = end;
	}

	AL.buffer.metaByteLen = metadata.length * AL.MD_ELEM_SIZE_BYTES;

	// See if we need to grab some memory.
	if (!AL.buffer.ptr) {
		AL.buffer.size = AL.BUFFER_PARAM_SIZE_BYTES + AL.buffer.metaByteLen + AL.buffer.blobByteLen;
		AL.freeBuffer();
		AL.buffer.ptr = Module._malloc(AL.buffer.size);
	}

	// Write out prefix data
	AL.writeBufferWord(AL.buffer.ptr, AL.BUFFER_PARAM_SLICE_OFFSET, 0);

	AL.writeBufferWord(AL.buffer.ptr, AL.BUFFER_PARAM_SLICE_SIZE,
		AL.buffer.blobByteLen);

	AL.writeBufferWord(AL.buffer.ptr, AL.BUFFER_PARAM_SLICES_LEFT, 0);

	AL.writeBufferWord(AL.buffer.ptr, AL.BUFFER_PARAM_MD_LEN,
		AL.buffer.metaByteLen);

	// Write out blob metadata
	if (!AL.buffer.hasMetaData) {
		AL.binifyMetadata(AL.buffer.ptr + AL.BUFFER_PARAM_SIZE_BYTES,
			metadata);
		AL.buffer.hasMetaData = true;
	}

	// Minor gripe: it's nice that we can slice out sections of blobs.
	// What would be just as nice is if we could concatenate them
	// without having to resort to semi-hacks like this...
	let offset = AL.BUFFER_PARAM_SIZE_BYTES + AL.buffer.metaByteLen;
	for (let i = 0; i < blobBuff.length; ++i) {
		let blobBytes = new Uint8Array(blobBuff[i]);

		Module.writeArrayToMemory(
			blobBytes,
			AL.buffer.ptr + offset
		);

		offset += blobBytes.length;
	}
}



//---------------------------
// Slices
//---------------------------
//
// When a bundle is first loaded,
// each slice range of the blob
// is precomputed and stored;
// the client can then query the
// next slice as necessary

// metadata contains the actual files on disk,
// whereas files is the list we've received from the
// client to fetch - this requested list may or may _not_
// have extensions for each name.
AL.addSliceMeta = function(metadata, blobSize, files) {
	let accumSize = 0;

	let isShader = false;

	for (let k = 0; k < metadata.files.length; ++k) {
		if (metadata.files[k].filename.indexOf('.shader') !== -1) {
			isShader = true;
			break;
		}
	}

	// Every shader is significant, because the shaders themselves usually
	// aren't designed for a single map.
	if (isShader) {
		AL.sliceMeta.push({
			start: metadata.files[0].start,
			end: metadata.files[metadata.files.length - 1].end
		});

		return metadata.files;
	} else {
		// Other files are map dependent, so we are selective here.

		if (!files) {
			throw 'Expected a pipe delimited list of file paths for the bundle';
		}

		files = files.split('|');

		if (AL.CONTENT_PIPELINE_MSG) {
			console.log('Fetching ', files.length, ' files...');
		}

		let undefCounter = 0;

		// Apparently chrome doesn't copy strings on assignment. Even if it
		// doesn't have this issue anymore, the fact that different browsers
		// have differing semantics like this is bad mmk.
		// https://stackoverflow.com/a/31733628
		function cloneString(toClone) {
			return (' ' + toClone).slice(1);
		}

		function stripExt(filename) {
			let copy = cloneString(filename);
			if (copy.indexOf('.') !== -1) {
				let c = copy.length - 1;
				while (copy.charAt(c) !== '.') {
					c--;
				}
				copy = copy.substring(0, c);
			}
			return copy;
		}

		for (let i = 0; i < metadata.files.length && undefCounter < files.length; ++i) {
			for (let j = 0; j < files.length; ++j) {
				// Yeah, it's only a string comparison but it's an O(1) check vs O(N) so w/e
				if (!files[j])
					continue;

				// Strip both extensions, given that some image filenames
				// end in .tga but only have .jpg equivalents
				let f0 = stripExt(metadata.files[i].filename);
				let f1 = stripExt(files[j]);

				if (f0.valueOf() === f1.valueOf()) {
					accumSize += metadata.files[i].end - metadata.files[i].start;

					AL.sliceMeta.push({
						start: metadata.files[i].start,
						end: metadata.files[i].end,
						filename: cloneString(metadata.files[i].filename)
					});

					if (AL.CONTENT_PIPELINE_MSG) {
						console.log('Caught: ', files[j], '. Slice Count: ',
							AL.sliceMeta.length);
					}

					delete files[j];
					undefCounter++;
				}
			}
		}

		// Here we're done: we only care about requested files that exist in
		// the bundle at this point. If we find that enough images
		// supersede our memory constraints then the implementation
		// will be altered.
		metadata.files = null;

		const SEGMENT_SIZE = Math.min((TOTAL_MEMORY * 0.6)|0, blobSize);

		if (accumSize > SEGMENT_SIZE) {
			if (AL.CONTENT_PIPELINE_MSG) {
				console.log('accumSize > SEGMENT_SIZE exception thrown!');
			}
			throw 'accumSize too large: ' + accumSize + '. Must not exceed: '
				+ SEGMENT_SIZE;
		}

		return AL.sliceMeta;
	}
}

AL.nextSlice = function(msb, metaFiles) {
	AL.allocMem(
		AL.buffer.loader,
		metaFiles
	);

	AL.finishTime(
		msb,
		AL.buffer.loader,
		AL.buffer.metaByteLen,
		AL.buffer.blobByteLen
	);

	AL.callLoaderCB(AL.buffer.loader, AL.buffer.ptr, AL.buffer.size);
}

AL.loadFinished = function(loader) {
	if (loader.params.map) {
		let msb = AL.startTime();

		AL.clearBuffer();

		AL.buffer.loader = loader;

		let metaFiles = AL.addSliceMeta(
			AL.buffer.loader.packageRef.metadata,
			AL.buffer.loader.packageRef.blob.size,
			AL.buffer.loader.params.path // is a list of paths, not one "path"
		);

		AL.nextSlice(msb, metaFiles, AL.buffer.loader.params.path);
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

// params "path" and "pathLength" can represent one of two types of strings:
// 1) a single file path which is going to be sent to the callback after the
// bundle's been mounted in the worker filesystem.
// 2) a pipe-delimited list of filepaths which are sent to the global Bundle instance
// in the client
AL.fetchBundleAsync = function(bundleName, callback, path, pathLength, port, map) {
	var loader = new AL.BundleLoader(
		AL.getMaybeCString(bundleName),
		{
			proxy: callback,
			path: AL.getMaybeCString(path),
			size: pathLength,
			port: port,
			map: map || false
		}
	);

	loader.load();
}

self.fetchBundleAsync = AL.fetchBundleAsync;
self.unmountPackages = AL.unmountPackages;
self.mountPackages = AL.mountPackages;
self.clearBuffer = AL.clearBuffer;

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
		 var stack = stackSave();
		 dynCall('vii', callbackPtr, [data, size]);
		 stackRestore(stack);
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
