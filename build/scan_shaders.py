import os

shader_names = [
	"textures/gothic_trim/pitted_rust3",
	"textures/gothic_block/blocks19",
	"textures/gothic_block/killblock",
	"textures/gothic_floor/xstairtop4",
	"textures/gothic_floor/xstepborder3",
	"textures/gothic_trim/km_arena1tower6",
	"textures/gothic_trim/baseboard08_d",
	"textures/gothic_wall/iron01_e",
	"textures/gothic_wall/iron01_c3",
	"noshader",
	"textures/common/caulk",
	"textures/gothic_floor/largerblock3b2",
	"textures/gothic_block/blocks18cbloodhead",
	"textures/gothic_block/killblock_j2",
	"textures/base_floor/metalbridge06",
	"textures/gothic_trim/baseboard08_e",
	"textures/gothic_block/blocks18c",
	"textures/organics/trainpart12C",
	"textures/gothic_trim/metalsupport4g",
	"textures/gothic_block/x1blocks",
	"textures/gothic_trim/newskull",
	"textures/skies/tim_dm3_red",
	"textures/gothic_trim/pitted_rust",
	"textures/skin/skin6move",
	"textures/gothic_trim/metalsupsolid",
	"textures/gothic_floor/goopq1metal7_98e",
	"textures/skin/skin6",
	"textures/gothic_block/killblock_i3",
	"textures/gothic_block/gkc19a",
	"textures/gothic_ceiling/woodceiling1b_dark",
	"textures/gothic_trim/baseboard09_g",
	"textures/gothic_block/blocks15",
	"textures/gothic_ceiling/ceilingtech_big",
	"textures/gothic_trim/baseboard09_l",
	"textures/common/clip",
	"textures/organics/wire02_f2",
	"textures/common/hint",
	"textures/gothic_block/gkc18b",
	"textures/liquids/lavafloor",
	"textures/gothic_trim/pitted_rust2_trans",
	"textures/gothic_block/gkc15b",
	"textures/gothic_block/gkc14e",
	"textures/gothic_block/gkc15c",
	"textures/gothic_block/gkc20a",
	"textures/gothic_trim/q_waste1",
	"textures/gothic_light/ironcrosslt2_5000",
	"textures/gothic_light/ironcrosslt2_3000",
	"textures/gothic_block/killblock_i4",
	"textures/gothic_block/killblock_i4b",
	"textures/gothic_light/ironcrosslt2_10000",
	"textures/gothic_light/pentagram_light1_5K",
	"textures/gothic_block/blocks18cblood",
	"textures/gothic_trim/metalsupport4b",
	"textures/gothic_block/blocks18b",
	"textures/liquids/lavahellflat_400",
	"textures/common/nodraw",
	"textures/sfx/flame1side",
	"textures/skin/nibbles",
	"textures/sfx/bouncepad01block18b",
	"textures/gothic_block/killtrim",
	"textures/gothic_light/ironcrosslt2_1000",
	"textures/gothic_block/dark_block",
	"textures/base_floor/rusty_pentagrate",
	"textures/base_floor/proto_rustygrate",
	"textures/common/weapclip",
	"textures/gothic_door/xian_dm3arch",
	"textures/base_trim/dirty_pewter",
	"textures/gothic_door/tim_dmarch02",
	"textures/gothic_door/tim_dmarch01",
	"textures/gothic_trim/metaldemonkillblock",
	"textures/gothic_trim/km_arena1tower7",
	"textures/gothic_trim/km_arena1tower2",
	"textures/gothic_trim/metalsupport4h",
	"textures/gothic_trim/pitted_rust2",
	"textures/sfx/flame1dark",
	"textures/gothic_floor/metalbridge06",
	"textures/gothic_trim/tower_front",
	"textures/sfx/flame1",
	"textures/base_trim/techborder",
	"textures/common/donotenter",
	"textures/skin/skin1",
	"textures/gothic_block/killtrim_trans",
	"textures/skin/skin5_trans",
	"textures/skin/skin6_trans",
	"textures/skin/pjwal2k",
	"textures/sfx/flame2",
	"models/mapobjects/gratelamp/gratetorch2",
	"models/mapobjects/gratelamp/gratetorch2b",
	"models/mapobjects/wallhead/wallhead02",
	"models/mapobjects/Skull/skull",
	"models/mapobjects/skull/ribcage",
	"models/mapobjects/teleporter/energy",
	"models/mapobjects/teleporter/teleporter",
	"models/mapobjects/teleporter/teleporter_edge",
	"models/mapobjects/teleporter/pad",
	"models/mapobjects/teleporter/transparency",
	"models/mapobjects/teleporter/widget",
	"models/mapobjects/storch/storch",
	"models/mapobjects/storch/storch_tall",
	"flareShader",
	"textures/common/trigger"
]

in_shader_files = {}
only_images = {}

print ("total names: %i" % len(shader_names))

def search_for_shader_in_file(shader, file):
    with open(file, 'r') as handle:
        for line in handle:
            if line == shader + "\n":
                return True
    return False

def search_for_shader(dir):
    # traverse root directory, and list directories as dirs and files as files
    for root, dirs, files in os.walk("./asset/" + dir):
        path = root.split(os.sep)
        for name in shader_names:
            for file in files:
                if dir == "scripts":
                    if search_for_shader_in_file(name, os.path.join(root, file)):
                        in_shader_files[name] = file
                    #    break
                else:
                    p = os.path.join(os.sep.join(path[2:]), file)
                    #print(p)
                    if p.startswith(name + "."):
                        only_images[name] = file
                        #break

search_for_shader("scripts")
search_for_shader("")

def print_names(title, map):
    print(title)
    counter = 0
    for entry in sorted(map.keys()):
        print("\t[%i] %s => %s" % (counter, entry, map[entry]))
        counter += 1

print_names("Shader File Matches", in_shader_files)
print_names("Image File Matches", only_images)

dupes = {}

for key in in_shader_files:
    if key in only_images:
        dupes[key] = []
        dupes[key].append(in_shader_files[key])

for key in only_images:
    if key in in_shader_files:
        dupes[key].append(only_images[key])

print_names("Union", dupes)

print("len(Shader File Matches) + len(Image File Matches) = %i" \
    % (len(in_shader_files) + len(only_images)))
