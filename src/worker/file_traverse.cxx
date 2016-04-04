#include <emscripten.h>
#include <stdio.h>
#include <string.h>
#include <vector>

void TestFile( unsigned char* path )
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
			console.log( 'Loading bundles...' );
			var names = [
				'sprites',
				'gfx',
				'scripts',
				'maps',
				'textures',
				'env'
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
						console.log( 'status 200; writing...' );
						param[extName['param']] = xhr.request;
					}
				}
			}

			for (var i = 0; i < names.length; ++i) {
				for (var n in exts) {
					extName = n;
					xhr = new XMLHttpRequest();
					var url = 'http://localhost:6931/bundle/' + names[i] + n;
					console.log( 'Loading ', url, '...' );
					xhr.open('GET', url, false);
					xhr.responseType = exts[extName]['type'];
					xhr.setRequestHeader('Access-Control-Allow-Origin', 'http://localhost:6931');
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

void ReadFile( char* path, int size )
{
	puts( "Worker: ReadFile entering" );

	CheckFilesMounted();

	FILE* f = fopen( path, "rb" );

	fseek( f, 0, SEEK_END );
	size_t count = ftell( f );
	fseek( f, 0, SEEK_SET );

	std::vector< unsigned char > buffer( ( count + 1 ) * sizeof( unsigned char ), 0 );
	fread( &buffer[ 0 ], sizeof( unsigned char ), count, f );
	fclose( f );

	emscripten_worker_respond( ( char* ) &buffer[ 0 ],
		buffer.size() * sizeof( buffer[ 0 ] ) );
}

void Traverse( char* directory, int size )
{
	puts( "Worker: Traverse entering" );

	( void )size;

	CheckFilesMounted();

	char errorMsg[ 128 ];
	memset( errorMsg, 0, sizeof( errorMsg ) );

	int ret = EM_ASM_ARGS(
		try {
			return Module.walkFileDirectory($0, $1, $2);
		} catch (e) {
			console.log(e);
			throw e;
		}
	, directory, TestFile, errorMsg );

	if ( !ret )
		printf( "[WORKER ERROR]: %s\n", errorMsg );

	emscripten_worker_respond( NULL, 0 );
}

} // extern "C"
