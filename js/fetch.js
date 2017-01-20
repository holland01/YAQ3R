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

// If we choose to "map" directly into memory then we just use a
// custom binary format: everything becomes a simple memory access
//---------------
// Format
//--------------
// [0] 4 byte int -> start (start offset for the segment in the blob)
// [1] 4 byte int -> end (end offset for the segment in the blob)
// [2] string [64] -> filepath (path of the file; used when a pathname is passed)

AL.MD_ELEM_START_OFFSET = 0;
AL.MD_ELEM_END_OFFSET = 4;
AL.MD_ELEM_PATH_OFFSET = 8;
AL.MD_ELEM_PATH_MAX_SIZE = 64;

AL.MD_ELEM_SIZE_BYTES = 4 * 2 + AL.MD_ELEM_PATH_MAX_SIZE;

AL.binifyMetadata = function(buffer, metadata) {
	

	for (let file = 0; file < metadata.files.length; file++) {
		if (metadata.files[file].filename.length >= AL.MD_ELEM_PATH_MAX_SIZE) {
			throw 'File found with excessive large size: '
				+ metadata.files[file].filename +
			'; size: ' + metadata.files[file].filename.length;
		}

		let elem = buffer + file * AL.MD_ELEM_SIZE_BYTES;

		// write the start offset
		HEAP32[(elem + AL.MD_ELEM_START_OFFSET) >> 2] = metadata.files[file].start;
		// write the end offset
		HEAP32[(elem + AL.MD_ELEM_END_OFFSET) >> 2] = metadata.files[file].end;
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

AL.loadFinished = function(loader) {
	let pbuf = 0;
	let pbufLen = 0;
	let msb = null;

	if (loader.params.map) {
		if (AL.CONTENT_PIPELINE_MSG) {
			msb = Date.now();
		}

		let fileReader = new FileReaderSync();
		let readbuf = fileReader.readAsArrayBuffer(loader.packageRef.blob);

		// first 4 bytes represents the length of the buffer segment (we have 2 of them)
		const METADATA_BYTELEN = 4 + loader.packageRef.metadata.files.length * AL.MD_ELEM_SIZE_BYTES;
		const BLOB_BYTELEN = 4 + readbuf.byteLength;

		pbufLen = METADATA_BYTELEN + BLOB_BYTELEN;
		pbuf = Module._malloc(pbufLen);

		HEAP32[pbuf >> 2] = METADATA_BYTELEN - 4;

		AL.binifyMetadata(pbuf + 4, loader.packageRef.metadata);

		if (AL.CONTENT_PIPELINE_MSG) {
			console.log(
				"Load Time: ",
				Date.now() - msb,
				"; ",
				loader.bundle, "; ",
				loader.params.path, "mdbin size: ",
				METADATA_BYTELEN - 4,
				"; Blob size: ",
				BLOB_BYTELEN - 4
			);
		}

		HEAP32[(pbuf + METADATA_BYTELEN) >> 2] = BLOB_BYTELEN - 4;

		let readbufu8 = new Uint8Array(readbuf);

		Module.writeArrayToMemory(
			readbufu8,
			pbuf + METADATA_BYTELEN + 4
		);
	}
	else
	{
		AL.mountPackages([loader.packageRef]);
		if (loader.params.path) {
			pbuf = Module._malloc(loader.params.size + 1);
			/*
			 * NOTE: writeStringToMemory creates an additional array,
			 * which is where the null term will be added if at all;
			 * every element is then copied into pbuf.
			 * So, if there is a null term (which there almost certainly is),
			 * then pbuf's extra byte will suffice.
			 */
			Module.writeStringToMemory(loader.params.path, pbuf, false);
			pbufLen = loader.params.size;
		}
	}

	var stack = Runtime.stackSave();

	Runtime.dynCall('vii',
			loader.params.proxy,
			[pbuf,
			 pbufLen]);
	Runtime.stackRestore(stack);

	if (pbuf && !loader.params.map) {
		Module._free(pbuf);
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
