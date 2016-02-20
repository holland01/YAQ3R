VERBOSE=1
DEBUG=1
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

COMMONFLAGS = -O2 -Wall -Wextra -pedantic -Werror \
 -Isrc -Isrc/extern -s SAFE_HEAP=1 -s ALLOW_MEMORY_GROWTH=1 \

DEBUGFLAGS = -Wno-unused-function -Wno-unused-variable\
 -Wno-missing-field-initializers -Wno-self-assign\
  -Wno-unused-value -Wno-dollar-in-identifier-extension\
  -Wno-unused-parameter

LDFLAGS = --emrun
LDO = -O0

ifdef DEBUG
  COMMONFLAGS := $(COMMONFLAGS) -g4 -s DEMANGLE_SUPPORT=1
  COMMONFLAGS := $(COMMONFLAGS) $(DEBUGFLAGS)
else
  LDO = -O2
endif

CFLAGS = $(COMMONFLAGS) -std=c99
CXXFLAGS = $(COMMONFLAGS) -std=c++14

DEPDIR = deps

all: $(BINFILE)

ifeq ($(MAKECMDGOALS),)
-include Makefile.dep
endif
ifneq ($(filter-out clean, $(MAKECMDGOALS)),)
-include Makefile.dep
endif

CC = emcc -v
CXX = em++ -v
CXXO = -O2

DEPFLAGS= -s USE_SDL=2

ifdef EM_ASSERTIONS
  COMMONFLAGS := $(COMMONFLAGS) -s ASSERTIONS=2
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
	$(Q)$(CXX) $(LDFLAGS) $(OFILES) $(LDO) ~/.emscripten_cache/ports-builds/sdl2/libsdl2.bc -o $@ --preload-file emscripten_asset@
clean:
	$(E) Removing files
	$(Q)rm -rf obj/
	$(Q)rm -f $(BINFILE) Makefile.dep
	$(Q)mkdir obj
