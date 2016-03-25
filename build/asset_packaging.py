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
        'path': 'maps',
        'compress': True
    },
    {
        'target':'env.data',
        'path': 'env',
        'compress': True
    },
    {
        'target':'gfx.data',
        'path': 'gfx',
        'compress': True
    },
    {
        'target':'textures.data',
        'path': 'textures',
        'compress': True
    },
    {
        'target':'sprites.data',
        'path': 'sprites',
        'compress': True
    },
    {
        'target':'scripts.data',
        'path': 'scripts',
        'compress': True
    },
]

import subprocess

FILE_PACKAGER = os.path.join(os.environ['EMSCRIPTEN'], 'tools', 'file_packager.py')

def package_list(list, target_prefix, path_origin_prefix, path_alias_prefix):
    global FILE_PACKAGER
    for f in list:
        args = ['python', FILE_PACKAGER]
        args.append(os.path.join(target_prefix, f['target']))
        args.append('--preload')
        args.append(os.path.join(path_origin_prefix, f['path'], '@', path_alias_prefix, f['path']))
        args.append('--no-heap-copy')
        args.append('--js-output=' + os.path.join(target_prefix, f['path'] + '.js'))
        if f['compress']:
            args.append('--lz4')
        subprocess.call(args)

package_list(map_files, TARGET_PREFIX, MAP_ORIGIN_PREFIX, MAP_ALIAS_PREFIX)
