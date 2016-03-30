#include <emscripten.h>
#include <stdio.h>
#include <string.h>

void ReadFile( unsigned char* path )
{
	char* strpath = ( char* )path;

	emscripten_worker_respond_provisionally( strpath, strlen( strpath ) );
}

static bool gFilesMounted = false;

static void CheckFilesMounted( void )
{
	if ( !gFilesMounted )
	{
		EM_ASM( {
			var names = [
				'bspData',
				'shader_gen',
				'sprites',
				'atlas_gen',
				'gfx',
				'q3bsp_texgen',
				'scripts',
				'gl',
				'maps',
				'texgen_effect_shader',
				'textures',
				'env',
				'drawLog'
			];
			var packages = [];
			var xhr = null;
			var extName = null;
			var exts = {
				'.data': {
					'type': 'blob',
					'param': 'blob'
				},
				'.js.metadata': {
					'type': 'json',
					'param': 'metadata'
				}
			};
			var param = {};
			function getData(evt) {
				console.log('XHR Ready State: ' + xhr.readyState
			+ '\nXHR Status: ' + xhr.status);
				if (xhr.readyState === XMLHttpRequest.DONE) {
					if (xhr.status === 200) {
						param[extName['param']] = xhr.request;
					}
				}
			}

			for (var i = 0; i < names.length; ++i) {
				for (var n in exts) {
					extName = n;
					xhr = new XMLHttpRequest();
					var url = 'http://localhost:6931/bundle/' + names[i] + n;
					xhr.open('GET', url, false);
					xhr.responseType = exts[extName['type']];
					xhr.setRequestHeader('Access-Control-Allow-Origin', 'http://localhost:6931');
					//xhr.setRequestHeader('Content-Type', exts[n]);
					xhr.addEventListener('load', getData);
					xhr.send();
				}
				packages.push(param);
			}

			FS.mkdir('/working');
			FS.mount(WORKERFS, {
				packages: packages
			}, '/working');
		} );

		gFilesMounted = true;
	}
}

extern "C" {

void Traverse( char* directory, int size )
{
	CheckFilesMounted();

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
		printf( "[WORKER ERROR]: %s\n", errorMsg );

	emscripten_worker_respond( NULL, 0 );
}

} // extern "C"
