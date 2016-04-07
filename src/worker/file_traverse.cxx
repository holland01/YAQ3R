#include <emscripten.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include "../js/global_func.h"

void TestFile( unsigned char* path )
{
	char* strpath = ( char* )path;

	emscripten_worker_respond_provisionally( strpath, strlen( strpath ) );
}

static bool gInitialized = false;

static void InitSystem( void )
{
	if ( !gInitialized )
	{
		const char* globalDef =
			EM_FUNC_WALK_FILE_DIRECTORY
			"self.walkFileDirectory = walkFileDirectory;";

		emscripten_run_script( globalDef );

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
			for (var i = 0; i < names.length; ++i) {
				var param = {};
				for (var n in exts) {
					var xhr = new XMLHttpRequest();
					var url = 'http://localhost:6931/bundle/' + names[i] + n;
					console.log( 'Loading ', url, '...' );
					xhr.open('GET', url, false);
					xhr.responseType = exts[n]['type'];
					xhr.setRequestHeader('Access-Control-Allow-Origin', 'http://localhost:6931');
					xhr.addEventListener('readystatechange', function(evt) {
						console.log('XHR Ready State: ' + xhr.readyState
							+ '\nXHR Status: ' + xhr.status);
						if (xhr.readyState === XMLHttpRequest.DONE) {
							console.log( 'status 200; writing...' );
							param[exts[n]['param']] = xhr.response;
						}
					});
					xhr.send();
				}
				packages.push(param);
			}

			FS.mkdir('/working');
			FS.mount(WORKERFS, {
				packages: packages
			}, '/working');
		} );

		gInitialized = true;
	}
}

extern "C" {

void ReadFile( char* path, int size )
{
	puts( "Worker: ReadFile entering" );

	InitSystem();

	FILE* f = fopen( path, "rb" );

	if ( f )
	{
		fseek( f, 0, SEEK_END );
		size_t count = ftell( f );
		fseek( f, 0, SEEK_SET );

		std::vector< unsigned char > buffer( ( count + 1 ) * sizeof( unsigned char ), 0 );
		fread( &buffer[ 0 ], sizeof( unsigned char ), count, f );
		fclose( f );

		emscripten_worker_respond( ( char* ) &buffer[ 0 ],
			buffer.size() * sizeof( buffer[ 0 ] ) );
	}
	else
	{
		emscripten_worker_respond( nullptr, 0 );
	}
}

void Traverse( char* directory, int size )
{
	puts( "Worker: Traverse entering" );

	( void )size;

	InitSystem();

	char errorMsg[ 128 ];
	memset( errorMsg, 0, sizeof( errorMsg ) );

	int ret = EM_ASM_ARGS(
		console.log('Does this work lol: ', self.walkFileDirectory);
		try {
			return self.walkFileDirectory($0, $1, $2);
		} catch (e) {
			console.log(e);
			throw e;
		}
		return 0;
	, directory, TestFile, errorMsg );

	if ( !ret )
		printf( "[WORKER ERROR]: %s\n", errorMsg );

	emscripten_worker_respond( NULL, 0 );
}

} // extern "C"
