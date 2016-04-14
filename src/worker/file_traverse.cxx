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

typedef void ( *callback_t )( char* data, int size );

static bool gInitialized = false;

static bool InitSystem( callback_t proxy, char* data, int size )
{
	if (!gInitialized)
	{
		const char* globalDef =
			EM_FUNC_WALK_FILE_DIRECTORY
			"self.walkFileDirectory = walkFileDirectory;\n"\
			"self.fetchBundleAsync = fetchBundleAsync;\n"\
			"self.xhrSnagFromName = xhrSnagFromName;";

		emscripten_run_script( globalDef );

		/*var names = [
			'sprites',
			'gfx',
			'scripts',
			'maps',
			'textures',
			'env'
		];*/

		EM_ASM_ARGS({
			// packagesRef refers to the local variable
			// defined further down.
			// We wait until we're finished with all of the package fetching
			// before a file system mount
			var lists = [
				[ 'sprites', 'gfx' ], // two folders per fetch
				[ 'scripts', 'maps' ],
				[ 'textures', 'env']
			];
			var fetchCount = 0;
			function onFinish(packagesRef) {
				fetchCount++;
				if (fetchCount === lists.length) {
					FS.mkdir('/working');
					FS.mount(WORKERFS,
						{packages: packagesRef},
					'/working');

					// May need to duplicate string memory, but since
					// it's already a pointer likely not...
					//var u8buf = intArrayFromString(p);
					//var pbuf = Module._malloc(u8buf.length);
					Module.writeArrayToMemory(u8buf, pbuf);
					var stack = Runtime.stackSave();
					iterate = !!Runtime.dynCall('iii', $0, /*[pbuf]*/ [$1, $2]);
					Runtime.stackRestore(stack);
					Module._free(pbuf);
				}
			}

			// Allocate extensions object here and here only,
			// to avoid unnecessary GC allocations
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

			var packages = [];

			for (var i = 0; i < lists.length; ++i) {
				fetchBundleAsync(lists[i], exts, packages, lists.length - i);
			}
		}, proxy, data, size);

		gInitialized = true;
		return false;
	}

	return true;
}

static void ReadFile_Proxy( char* path, int size )
{
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

extern "C" {

void ReadFile( char* path, int size )
{
	puts( "Worker: ReadFile entering" );

	if ( InitSystem( ReadFile_Proxy, path, size ) )
	{
		ReadFile_Proxy( path, size );
	}
}

void Traverse( char* directory, int size )
{
	puts( "Worker: Traverse entering" );

	( void )size;

	if( 0 )
	{
		InitSystem( nullptr, directory, size );
	}
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
