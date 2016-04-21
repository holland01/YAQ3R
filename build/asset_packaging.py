import os, shutil

MAP_ORIGIN_PREFIX = os.path.join('..', 'asset')
MAP_ALIAS_PREFIX = os.path.join('asset')
TARGET_PREFIX = os.path.join('..', 'bundle')

if os.path.exists(TARGET_PREFIX):
    shutil.rmtree(TARGET_PREFIX)

os.mkdir(TARGET_PREFIX)

COMPRESS_MAPS = False
map_files = [
    {
        'target':'maps.data',
        'name': 'maps',
        'compress': COMPRESS_MAPS
    },
    {
        'target':'env.data',
        'name': 'env',
        'compress': COMPRESS_MAPS
    },
    {
        'target':'gfx.data',
        'name': 'gfx',
        'compress': COMPRESS_MAPS
    },
    {
        'target':'textures.data',
        'name': 'textures',
        'compress': COMPRESS_MAPS
    },
    {
        'target':'sprites.data',
        'name': 'sprites',
        'compress': COMPRESS_MAPS
    },
    {
        'target':'scripts.data',
        'name': 'scripts',
        'compress': COMPRESS_MAPS
    },
]

LOG_ORIGIN_PREFIX = os.path.join('..', 'emscripten_asset', 'log')
LOG_ALIAS_PREFIX = os.path.join('log')

log_files = [
    {
        'target': 'atlas_gen.data',
        'name': 'atlas_gen',
        'ext': '.txt',
        'compress': False
    },
    {
        'target': 'bspData.data',
        'name': 'bspData',
        'ext': '.log',
        'compress': False
    },
    {
        'target': 'drawLog.data',
        'name': 'drawLog',
        'ext': '.log',
        'compress': False
    },
    {
        'target': 'shader_gen.data',
        'name': 'shader_gen',
        'ext': '.txt',
        'compress': False
    },
    {
        'target': 'gl_log.data',
        'name': 'gl',
        'ext': '.log',
        'compress': False
    },
    {
        'target': 'q3bsp_texgen.data',
        'name': 'q3bsp_texgen',
        'ext': '.txt',
        'compress': False
    },
    {
        'target': 'texgen_effect_shader.data',
        'name': 'texgen_effect_shader',
        'ext': '.txt',
        'compress': False
    }
]

import subprocess

FILE_PACKAGER = os.path.join(os.environ['EMSCRIPTEN'], 'tools', 'file_packager.py')

def package_list(list, target_prefix, path_origin_prefix, path_alias_prefix):
    global FILE_PACKAGER
    for f in list:
        path = f['name']
        if 'ext' in f:
            path += f['ext']
        args = ['python', FILE_PACKAGER]
        args.append(os.path.join(target_prefix, f['target']))
        args.append('--preload')
        args.append(os.path.join(path_origin_prefix, path))
        args.append('--no-heap-copy')
        args.append('--separate-metadata')
        args.append('--js-output=' + os.path.join(target_prefix, f['name'] + '.js'))
        if f['compress']:
            args.append('--lz4')
        subprocess.call(args)

package_list(map_files, TARGET_PREFIX, MAP_ORIGIN_PREFIX, MAP_ALIAS_PREFIX)
#package_list(log_files, TARGET_PREFIX, LOG_ORIGIN_PREFIX, LOG_ALIAS_PREFIX)
