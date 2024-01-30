SHELLTYPE := posix
ifeq (.exe,$(findstring .exe,$(ComSpec)))
  SHELLTYPE := msdos
endif

ifndef c
	c=zig cc
endif

ifndef cpp
	cpp=zig c++
endif

ifndef target
	target=$(shell $(cpp) -dumpmachine)
endif

ifndef config
  config=debug
endif

.PHONY: build link prebuild dirs clean

INCLUDES := -Iinclude -I../citruscore/include -I../libs/spdlog/include -I../libs/glm -I../libs/boost/include -I../libs -I../libs/imgui -I../libs/stb -I../libs/assimp/include -I../assimp_generated
DEFINES := -DGLFW_INCLUDE_NONE
LIBS := -l:libglad_gl3.a
LIBDIRS := -L/usr/lib64 -Lbuild/$(target)/$(config)/link
MISCOPTS := -std=gnu++20 -c
CFGFLAGS :=

SRCDIR := src
OBJDIR := build/$(target)/$(config)/obj
BINDIR := build/$(target)/$(config)/bin
BIN := $(BINDIR)/libcitrusglglfw.so

ifeq ($(config),debug)
	CFGFLAGS += -g -DCE_ENV_DEBUG
else ifeq ($(config),release)
	CFGFLAGS += -O2 -s -DCE_ENV_RELEASE
endif

ifeq 

OPTS := $(INCLUDES) $(DEFINES) $(LIBDIRS) $(LIBS) $(MISCOPTS) $(CFGFLAGS)

OBJECTS := $(OBJDIR)/Window.o

default: build

dirs:
ifeq (posix,$(SHELLTYPE))
	@mkdir -p $(OBJDIR)
else
	@mkdir $(subst /,\\,$(OBJDIR))
endif
ifeq (posix,$(SHELLTYPE))
	@mkdir -p $(BINDIR)
else
	@mkdir $(subst /,\\,$(BINDIR))
endif
ifeq (posix,$(SHELLTYPE))
	@mkdir -p build/$(target)/$(config)/link
else
	@mkdir $(subst /,\\,build/$(target)/$(config)/link)
endif

clean:
ifeq (posix,$(SHELLTYPE))
	@rm -rf build
else
	@if exist $(build) rmdir /s /q $(build)
endif 

prebuild:
	@echo "Citrus GLFW+OpenGL Backend build: Compile Objects (1/2)"

link:
	@echo ""
	@echo "Citrus GLFW+OpenGL Backend build: Link Library (2/2)"
	@echo "Linking '$(BIN)'..."
ifeq (posix,$(SHELLTYPE))
	@mkdir -p $(BINDIR)/tmp
else
	@mkdir $(subst /,\\,$(BINDIR)/tmp)
endif
	@$(cpp) $(OBJECTS) $(CFGFLAGS) $(LIBDIRS) $(LIBS) $(subst -c,,$(MISCOPTS)) -o $(BIN)

build: dirs prebuild $(OBJECTS) link
	@:

# Files
###########

$(OBJDIR)/Window.o: $(SRCDIR)/Window.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<