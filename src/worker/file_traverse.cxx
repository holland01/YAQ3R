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
			"function xhrSnagFromName(exts, key, name, next, param,\n"\
				"packages) {\n"\
				"var xhr = new XMLHttpRequest();\n"\
				"var url = 'http://localhost:6931/bundle/' + name + key;\n"\
				"console.log('Loading ', url, '...');\n"\
				"xhr.open('GET', url);\n"\
				"xhr.responseType = exts[key]['type'];\n"\
				"xhr.setRequestHeader('Access-Control-Allow-Origin','http://localhost:6931');\n"\
				"xhr.addEventListener('readystatechange', function(evt) {\n"\
					"console.log('XHR Ready State: ' + xhr.readyState + '\nXHR Status: ' + xhr.status);\n"\
					"if (xhr.readyState === XMLHttpRequest.DONE) {\n"\
						"console.log('status 200; writing...');\n"\
						"param[exts[key]['param']] = xhr.response;\n"\
						"if (next.length > 0) {\n"\
							"f = next.shift();\n"\
							"f(next, exts, name, packages);\n"\
						"}\n"\
					"}\n"\
				"});\n"\
				"xhr.send();\n"\
			"}\n"
			"function fetchBundleAsync(names, finished, exts, packages) {\n" \
				"console.log('Loading bundles with the following names: ', JSON.stringify(names));\n" \
				"for (var i = 0; i < names.length; ++i) \n{" \
					"var param = {};\n" \
					"var funcEvents = \n[" \
						"function(next, exts, name, packcages) \n{" \
							"xhrSnagFromName(exts, '.js.metadata', names[i], param, next, packages);\n" \
						"},\n" \
						"function(next, exts, name, packages) {\n" \
							"packages.push(param);\n" \
							"if (i === names.length - 1) {\n" \
								"finished(packages);\n" \
							"}\n" \
						"}\n" \
					"];\n" \
					"xhrSnagFromName(exts, '.data', names[i], param, funcEvents, packages);\n" \
				"}\n" \
			"}\n" \
			"function beginFetch(proxy, data, size) {\n" \
				"var bundles = [['sprites', 'gfx'],['scripts', 'maps'],['textures', 'env']];\n" \
				"var fetchCount = 0;\n" \
				"function onFinish(packagesRef) {\n" \
					"fetchCount++;\n" \
					"if (fetchCount === bundles.length) {\n" \
						"FS.mkdir('/working');\n" \
						"FS.mount(WORKERFS, { packages: packagesRef }, '/working');\n" \
						"var stack = Runtime.stackSave();\n" \
						"Runtime.dynCall('vii', proxy, [data, size]);\n" \
						"Runtime.stackRestore(stack);\n" \
					"}\n" \
				"}\n" \
				"var exts = {\n" \
					"'.data': {\n" \
						"'type': 'blob',\n" \
						"'param': 'blob'\n" \
					"},\n" \
					"'.js.metadata': {\n" \
						"'type': 'json',\n" \
						"'param': 'metadata'\n" \
					"}\n" \
				"};\n" \
				"var packages = [];\n" \
				"for (var i = 0; i < bundles.length; ++i) {\n" \
					"fetchBundleAsync(bundles[i], exts, packages, bundles.length - i);\n" \
				"}\n" \
			"}\n" \
			EM_FUNC_WALK_FILE_DIRECTORY \
			"self.walkFileDirectory = walkFileDirectory;\n"\
			"self.fetchBundleAsync = fetchBundleAsync;\n"\
			"self.xhrSnagFromName = xhrSnagFromName;\n"\
			"self.beginFetch = beginFetch;";

		emscripten_run_script( globalDef );

		// packagesRef refers to the local variable
		// defined further down.
		// We wait until we're finished with all of the package fetching
		// before a file system mount

		// Allocate extensions object here and here only,
		// to avoid unnecessary GC allocations

		// two folders per fetch

		// May need to duplicate string memory, but since
		// it's already a pointer likely not...
		//var u8buf = intArrayFromString(p);
		//var pbuf = Module._malloc(u8buf.length);
		EM_ASM_({
			self.beginFetch($0, $1, $2);
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
