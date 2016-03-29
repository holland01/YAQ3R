#include <emscripten.h>
#include <stdio.h>
#include <string.h>

void ReadFile( unsigned char* path )
{
	const char* strpath = ( const char* )path;

	emscripten_respond_provisionally( strpath, strlen( strpath ) );
}

extern "C" {

void Traverse( char* directory, int size )
{
	char errorMsg[ 128 ];
	memset( errorMsg, 0, sizeof( errorMsg ) );

	int ret = EM_ASM_ARGS( {

		var path = UTF8ToString($0);
		var lookup = FS.lookupPath(path);
		if (!lookup) {
			stringToUTF8('Path given ' + path + ' could not be found.', $2, 128);
			return 0;
		}

		var root = lookup.node;
		var iterate = true;

		function traverse(node) {
			if (!iterate) return;

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
	}, directory, ReadFile, errorMsg );

	if ( !ret )
		MLOG_ERROR( "%s", errorMsg );
}

} // extern "C"
