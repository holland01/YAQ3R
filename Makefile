VERBOSE=1
EM_ASSERTIONS=1

ifdef VERBOSE
	Q =
	E = @true
else
	Q = @
	E = @echo
endif

LFORMAT = o

CFILES := $(shell find src -mindepth 1 -maxdepth 4 -name "*.c")
CXXFILES := $(shell find src -mindepth 1 -maxdepth 4 -name "*.cpp")

INFILES := $(CFILES) $(CXXFILES)

OBJFILES := $(CXXFILES:src/%.cpp=%) $(CFILES:src/%.c=%)
DEPFILES := $(CXXFILES:src/%.cpp=%) $(CFILES:src/%.c=%)
OFILES := $(OBJFILES:%=obj/%.$(LFORMAT))

TARGET_NAME := bspviewer

BINFILE = $(TARGET_NAME).html

FILE_TRAVERSE_CFLAGS := --no-entry -I./src -fno-inline-functions -O0 -std=c++14 
FILE_TRAVERSE_IN := src/worker/file_traverse.cxx
FILE_TRAVERSE_EXPORT_FUNCTIONS := -s "EXPORTED_FUNCTIONS=['_ReadShaders', '_ReadMapFile_Begin', '_ReadMapFile_Chunk', '_ReadImage', '_MountPackage', '_UnmountPackages']"
FILE_TRAVERSE_OPTS :=\
	-s WASM=1 -s BUILD_AS_WORKER=1 -s TOTAL_MEMORY=234881024 -s STB_IMAGE=1\
	-lworkerfs.js
FILE_TRAVERSE_OUT := worker/file_traverse.wasm

COMMONFLAGS = -Wall -Wextra -pedantic -Werror \
 -Wno-dollar-in-identifier-extension \
 -Wno-unused-function \
 -Isrc -Isrc/extern -s SAFE_HEAP=1 \
 -DGL_ATLAS_EGL -DEMSCRIPTEN

ifdef DEBUG_RELEASE
	COMMONFLAGS := $(COMMONFLAGS) -DDEBUG_RELEASE
endif

DEBUGFLAGS = -DDEBUG -Wno-unused-function -Wno-unused-variable\
 -Wno-missing-field-initializers -Wno-self-assign\
  -Wno-unused-value -Wno-dollar-in-identifier-extension\
  -Wno-unused-parameter -g

LDFLAGS = --emrun --profiling-funcs -lworkerfs.js -s WASM=1
LDO = -s LZ4=1 -s DEMANGLE_SUPPORT=1 -s TOTAL_MEMORY=536870912

ifdef PRELOAD_ALL_ASSETS
	LDFLAGS := $(LDFLAGS) --preload-file asset_preload_all@
else
	LDFLAGS := $(LDFLAGS) #--preload-file asset_preload_log@
	COMMONFLAGS := $(COMMONFLAGS) -DASYNC_FILE_TRAVERSAL
endif

ifdef DEBUG
  COMMONFLAGS := $(COMMONFLAGS) -O0 $(DEBUGFLAGS)
  LDO := $(LDO) -O0
else
  LDO := $(LDO) -O2
  COMMONFLAGS := $(COMMONFLAGS) -O2
endif

ifdef EM_ASSERTIONS
  COMMONFLAGS := $(COMMONFLAGS) -s ASSERTIONS=1
endif

CFLAGS = $(COMMONFLAGS) -std=c++14 -s WASM=1
CXXFLAGS = $(CFLAGS)

DEPDIR = deps

all: $(BINFILE)

ifeq ($(MAKECMDGOALS),)
-include Makefile.dep
endif
ifneq ($(filter-out clean, $(MAKECMDGOALS)),)
-include Makefile.dep
endif

# Add -v to both for verbosity
CC = em++
CXX = em++
CXXO =

ifdef DEBUG
  CXXO := $(CXXO) -O0
else
  CXXO := $(CXXO) -O2
endif

DEPFLAGS= -s USE_SDL=2

-include Makefile.local

.PHONY: clean all depend
.SUFFIXES:

obj/%.$(LFORMAT): src/%.c
	$(E) C-compiling $<
	$(Q)if [ ! -d `dirname $@` ]; then mkdir -p `dirname $@`; fi
	$(CC) -c $(CFLAGS) $(CXXO) $< $(DEPFLAGS) -o $@

obj/%.$(LFORMAT): src/%.cpp
	$(E) C++-compiling $<
	$(Q)if [ ! -d `dirname $@` ]; then mkdir -p `dirname $@`; fi
	$(CXX) -c $(CXXFLAGS) $(CXXO) $< $(DEPFLAGS) -o $@

#Makefile.dep: $(CFILES) $(CXXFILES)
#$(E)Depend
#$(Q)for i in $(^); do $(CXX) $(CXXFLAGS) -MM "$${i}" -MT obj/`basename $${i%.*}`.$(LFORMAT); done > $@


$(BINFILE): $(OFILES)
	$(E) The Path is this: $PATH
	$(E) Linking $@
	$(CXX) $(LDFLAGS) -s FORCE_FILESYSTEM=1 -s DISABLE_EXCEPTION_CATCHING=0 $(OFILES) $(LDO) $(EMSCRIPTEN)/cache/wasm/libSDL2.a -o $@ $(LDFLAGS)
	wasm-dis $(TARGET_NAME).wasm -o $(TARGET_NAME).wat

file_traverse: 
	$(CXX) $(FILE_TRAVERSE_CFLAGS) $(FILE_TRAVERSE_IN) $(FILE_TRAVERSE_EXPORT_FUNCTIONS) $(FILE_TRAVERSE_OPTS) -o $(FILE_TRAVERSE_OUT)

clean:
	$(E) Removing files
	$(Q)rm -rf obj/
	$(Q)rm -f $(BINFILE) $(TARGET_NAME).wasm Makefile.dep
	$(Q)mkdir obj
