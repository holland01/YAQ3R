VERBOSE=1
EM_ASSERTIONS=1

ifdef VERBOSE
	Q =
	E = @true
else
	Q = @
	E = @echo
endif

LFORMAT = bc

CFILES := $(shell find src -mindepth 1 -maxdepth 4 -name "*.c")
CXXFILES := $(shell find src -mindepth 1 -maxdepth 4 -name "*.cpp")

INFILES := $(CFILES) $(CXXFILES)

OBJFILES := $(CXXFILES:src/%.cpp=%) $(CFILES:src/%.c=%)
DEPFILES := $(CXXFILES:src/%.cpp=%) $(CFILES:src/%.c=%)
OFILES := $(OBJFILES:%=obj/%.$(LFORMAT))

BINFILE = bspviewer.html

COMMONFLAGS = -Wall -Wextra -pedantic -Werror \
 -Wno-dollar-in-identifier-extension \
 -Wno-unused-function \
 -Isrc -Isrc/extern -s SAFE_HEAP=1

ifdef DEBUG_RELEASE
	COMMONFLAGS := $(COMMONFLAGS) -DDEBUG_RELEASE
endif

DEBUGFLAGS = -DDEBUG -Wno-unused-function -Wno-unused-variable\
 -Wno-missing-field-initializers -Wno-self-assign\
  -Wno-unused-value -Wno-dollar-in-identifier-extension\
  -Wno-unused-parameter -g2

LDFLAGS = --emrun --profiling-funcs
LDO = -s LZ4=1 -s DEMANGLE_SUPPORT=1 -s TOTAL_MEMORY=536870912 #-s EMTERPRETIFY=1 \
 -s EMTERPRETIFY_ASYNC=1 -s EMTERPRETIFY_WHITELIST='["_main"]' -s 'EMTERPRETIFY_FILE="code.dat"'

ifdef PRELOAD_ALL_ASSETS
	LDFLAGS := $(LDFLAGS) --preload-file asset_preload_all@
else
	LDFLAGS := $(LDFLAGS) --preload-file asset_preload_log@
	COMMONFLAGS := $(COMMONFLAGS) -DASYNC_FILE_TRAVERSAL
endif

ifdef DEBUG
  COMMONFLAGS := $(COMMONFLAGS) -O0 $(DEBUGFLAGS)
  LDO := $(LDO) -O0
else
  LDO := $(LDO) -O2
  COMMONFLAGS := $(COMMONFLAGS) -O2
endif

CFLAGS = $(COMMONFLAGS) -std=c++14
CXXFLAGS = $(COMMONFLAGS) -std=c++14

DEPDIR = deps

all: $(BINFILE)

ifeq ($(MAKECMDGOALS),)
-include Makefile.dep
endif
ifneq ($(filter-out clean, $(MAKECMDGOALS)),)
-include Makefile.dep
endif

CC = em++ -v
CXX = em++ -v
CXXO = -O2

DEPFLAGS= -s USE_SDL=2

ifdef EM_ASSERTIONS
  COMMONFLAGS := $(COMMONFLAGS) -s ASSERTIONS=1
endif

-include Makefile.local

.PHONY: clean all depend
.SUFFIXES:

obj/%.$(LFORMAT): src/%.c
	$(E) C-compiling $<
	$(Q)if [ ! -d `dirname $@` ]; then mkdir -p `dirname $@`; fi
	$(Q)$(CC) $(CFLAGS) $(CXXO) $< $(DEPFLAGS) -o $@

obj/%.$(LFORMAT): src/%.cpp
	$(E) C++-compiling $<
	$(Q)if [ ! -d `dirname $@` ]; then mkdir -p `dirname $@`; fi
	$(Q)$(CXX) $(CXXFLAGS) $(CXXO) $< $(DEPFLAGS) -o $@

Makefile.dep: $(CFILES) $(CXXFILES)
	#$(E)Depend
	#$(Q)for i in $(^); do $(CXX) $(CXXFLAGS) -MM "$${i}" -MT obj/`basename $${i%.*}`.$(LFORMAT); done > $@


$(BINFILE): $(OFILES)
	$(E) The Path is this: $PATH
	$(E) Linking $@
	$(Q)$(CXX) $(LDFLAGS) $(OFILES) $(LDO) ~/.emscripten_cache/ports-builds/sdl2/libsdl2.bc -o $@ $(LDFLAGS)
clean:
	$(E) Removing files
	$(Q)rm -rf obj/
	$(Q)rm -f $(BINFILE) Makefile.dep
	$(Q)mkdir obj
