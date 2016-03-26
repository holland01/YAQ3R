import os, shutil

MAP_ORIGIN_PREFIX = os.path.join('..', 'emscripten_asset','asset','stockmaps')
MAP_ALIAS_PREFIX = os.path.join('asset', 'stockmaps')
TARGET_PREFIX = os.path.join('..', 'bundle')

if os.path.exists(TARGET_PREFIX):
    shutil.rmtree(TARGET_PREFIX)

os.mkdir(TARGET_PREFIX)

map_files = [
    {
        'target':'maps.data',
        'name': 'maps',
        'compress': True
    },
    {
        'target':'env.data',
        'name': 'env',
        'compress': True
    },
    {
        'target':'gfx.data',
        'name': 'gfx',
        'compress': True
    },
    {
        'target':'textures.data',
        'name': 'textures',
        'compress': True
    },
    {
        'target':'sprites.data',
        'name': 'sprites',
        'compress': True
    },
    {
        'target':'scripts.data',
        'name': 'scripts',
        'compress': True
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
        args.append(os.path.join(path_origin_prefix, path + '@' + path_alias_prefix, path))
        args.append('--no-heap-copy')
        args.append('--js-output=' + os.path.join(target_prefix, f['name'] + '.js'))
        if f['compress']:
            args.append('--lz4')
        subprocess.call(args)

package_list(map_files, TARGET_PREFIX, MAP_ORIGIN_PREFIX, MAP_ALIAS_PREFIX)
package_list(log_files, TARGET_PREFIX, LOG_ORIGIN_PREFIX, LOG_ALIAS_PREFIX)
