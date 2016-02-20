import fileinput

"""

,,,,,,,,,,,,,,,MM~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~MM$~~~~7MM?~MI::::IM
,,,,,,,,,,,,,,,M$~~~~~~~~~+~~~7M~~~~~~~~~~~~~~~~~~~~~~~~~~~~?M~~~~~~~~~~M:::::IM
,,,,,,,,,,,,::,M~~~~~~~~~~MZMIM~~~~~~~~~~~~~~~~~~~~~~~~~~~~~MDMM~~~~~~~~M:::::IM
,,,,,,,,::,+MZMD~~~~~~~~:~M~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~M~~~~~~~?M:::::MM
,,,,,,,:,,,MMMM~~~~~~~~~~~M~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~D?~~~~~~:OM:::::MD
,,,,,,,,,,,MM~~:~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~M~~~~~~~~~O8::::+MI
,,,,,,,,,,,MO~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~MDO+::+MI
,,,,,,,,:,,M~~~~~~~~~~~~~~~~~~~~$MMM~~~MMMM~~~=MM~~~~~~~~~~~~~~~~~~~~~~?=MM:~+MI
,,,,,,,::,~M~~~~~~~~~~~~~~~~~~~7MZ.MM?MM..MM8MMM?M~~~$?~~~~~~~~~~~~~~~~~MM=::+MI
,,,,,,,,,,8M~~~~~~~~~~~~~~IMMM8M7  .,7...   . .. OMMM:MM~=MMM~~~~~~~~~~IM::::+MI
,,,,,,,::~M~~~~~~~~~~~~~~~M$.MM  .. ... .  ... .  .  . MMM,.MMMMMMM~~~~MI::::+MI
,,,,,,,:,,M~~~~~~~~~~~~~~DM....                       .......,7~.MM~~?MMMM~:::MM
,,,,,,,:,,M~~~~~~~~~~~~~$M ...   ..              ................MM~~~~MD:::::MM
,,,,,,,,,,M~~~~~~~~~~~~~MM..... ...                            ..MM~~~IM::::::ZM
,,,,,,,,,,M~~~~~~~~~~~~+M7.........                            ..MM~~OM7::::::IM
,,,,,,,,,,M~~~~~~~~~~~~?M7..... ...... .....                   ..MM~ZMO:::::::IM
,,,,,,,,,,M~~~:~~$MMMMMMMMMMMMMMMN+.    . . .                  .OM~$M:::::::::7M
,,,,,,,,,,MMMMMMMMD~=~========?888MMMMMMM$:......      .    .. .MMIMN:::::::::+M
,,,,,,,,,MM... ...MM=====================+$MMMMMMMM?............MMM+O::::::::::M
,,,,,,,,MM.........MMMMMMMMMMM=~?7??~============78MMMMMMMZ~....MM:~$::::::::::M
,,,,,,,MM...ZMOM=..MM$M......M7==??77OMMMMMMMMMM===========?MMMMMMM8::~::::::::M
,,,,,,,MM ..Z,..IZ..MM..... .M7==========================?8OII?=====7MMMM8?~:::M
,,,,,,,MM.....  .M  MM..    .M7===?78MMMMO8II=====+MMMMI====7NMMMMMM====?DMM$::M
,,,,,,,,MM~... M.M  ....   ..8N===================MM .MM====~=======7O==?NMMO::M
,,,,,,,,,NM... MMN .       ..~M====~=============?MD..MM===DMMMMMMO?===IMO7::::M
,,,,,,,,:,:MZ.... ...      ...M====?778888OOO+==~MZ...MZ===========I7=8M~::::::M
,,,,,,,:,,,?MM$.  ...      ...MI================$M.. ~MI===OMMMMMMN==?MO:::::::M
,,,,,,,,,,,,MMM.            ...MMMMMMMMMMMMMMMMMM. . ~M7============~MM::::::::M
,,,,,,,,,,:,MM8.            ........=IIIIIIIII7,......MMMMMMMOI=====MM:::::::::M
,,,,,,,,,,,,MM  .           ..................................MMMMMMM::::::::::M
,,,,,,,,,,,:,MM.            . .......     .... . ............7MIM::::::::::::::M
,,,,,,,,,,,,,MM                      ..    .  . ...... ......MM,M+:::~:::::::::M
,,,,,,,,,,,,,MM                     ..MMMMDO8888O8MMMMMMO...MM,:MM:::::::::::::M
,,,,,,,,,,,,,MM                       M:.ZM$.8MD...MM .MD.. M:,,MM:::I:::::::::M
,,,,,,,,,,,,,MM                    .  .DM,. =..$MMMN MM....M7,,,7M~::M:::::::::M
,,,,,,,,:,,,,MM                     .. ..   .   ..  ..:M .ZM,,,,~MI::M:::::::::M
,,,,,,,,:,,,,M8..                                    .. ~:MO,,,,~MI::M:::::::::M
,,,,,,,,,,,,~M7                                      ....MM::,,,~MI::M:::::::::M
,,,,,,,,,,,,8M                                       .  NM,,,,,,~MI::M:::::::::M
,,,,,,,,,,:,MM..                                     . NM,,,,,,,~MI::M:::::::::M
:,,,,,,,,,,~M?                                   .....MM,,,,,,,,~MI::M:::::::::M
:,MMMMMMMMMMM.                                   ....DM::,::,,,,~MI::M:::::::::M
:,DM~~~~~~~M$.                                .. . .=M=,,,,,,,,,ZM~::M:::::::::M
:,,M~=~~~~?M..                               . . .8MM,,,,,,,,,,,MM:::M:::::::::M
,,,M~=~~~=MZ..                               . IMMM~,,,:,,,,,,,,MM:::M:::::::::M
,,,M~MM~~MM...                      ?MMMMMMMMM8=,,,,:,,,,,,,,,,,MM:::M:::::::::M
,,,M:~MMDM7...                      .  . .MM,,::,,::,,,:,,,,,,,,MM:::M:::::::::M
,,,M~~~DMMM......                     ...MM:,,,,,,,,,,,,,,,,,,,,MM~::M:::::::::M
,,,M~~=~~=MMM$...                     . .M7,,,:,,,,,,,,,,,,,,,,,MM~:~M:::::::::M
,,,M~~~~~~~~~+NMMN~.. .               ...MMMMZ,::,,,,,,,,,,,,,,,MM:::$=::::::::M
,,,M~~~~~~~=~~~=~DMMM?..               .$M~=~MMM:,,,,,,,,,,,,,,,MM:::I+::::::::M
"""

TM = str(104857600).encode('utf-8')

find_repl = { b"var TOTAL_MEMORY = Module['TOTAL_MEMORY'] || 16777216;\n": b"var TOTAL_MEMORY = " + TM + b";\n",
			  b"var TOTAL_MEMORY=Module[\"TOTAL_MEMORY\"]||16777216;": b"var TOTAL_MEMORY=" + TM + b";" }

position = 0

with open ("bspviewer.js", "rb") as f:
	with open ("bspviewer.bak.js", "wb") as f1:
		f1.write(f.read())

with open("bspviewer.js", "rb+") as f:
	found = False
	for line in f:
		if found:
			break
		line_length = len(line)
		for k in find_repl:
			if k in line:
				k_length = len(k)
				frepl_length = len(find_repl[k])
				print("\'%s\' found." % k)
				f.seek(position)
				for x in range(line_length):
					if line[x:x + k_length] == k:
						append = find_repl[k]
						if k_length > frepl_length:
							append += bytes(str.join('', [' ' for i in range(frepl_length, k_length)]), "utf8")

						line = line[:x] + append + line[x + len(k):]
						f.write(line)
						print("New Line: %s" % line)
						found = True
						break

					#new_line = find_repl[k] + bytes(str.join('', [' ' for i in range(len(find_repl[k]), len(line))]), "utf8")
					#print("Replacing \'%s\' with \'%s\' at bytes offset %i" % (line, new_line, position))

		position += line_length
