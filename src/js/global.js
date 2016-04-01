// $0 -> directory
// $1 -> callback
// $2 -> return error message (for caller)
Module.walkFileDirectory = function($0, $1, $2) {
	var path = UTF8ToString($0);
	var lookup = FS.lookupPath(path);
	if (!lookup) {
		stringToUTF8('Path given ' + path + ' could not be found.', $2, 128);
		return 0;
	}

	var root = lookup.node;
	var iterate = true;

	function traverse(node) {
		if (!iterate) {
			return;
		}

		var path = FS.getPath(node);
		var stat = FS.stat(path);
		if (FS.isFile(stat.mode)) {
			return;
		}

		for (var n in node.contents) {
			traverse(node.contents[n]);
			var p = FS.getPath(node.contents[n]);
			var u8buf = intArrayFromString(p);
			var pbuf = Module._malloc(u8buf.length);
			Module.writeArrayToMemory(u8buf, pbuf);
			var stack = Runtime.stackSave();
			iterate = !!Runtime.dynCall('ii', $1, [pbuf]);
			Runtime.stackRestore(stack);
			Module._free(pbuf);
		}
	}

	traverse(root);
	return 1;
};