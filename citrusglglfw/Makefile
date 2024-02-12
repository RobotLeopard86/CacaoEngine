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

INCLUDES := -Iinclude -I../citruscore/include -I../libs/spdlog/include -I../libs/glm -I../libs -I../libs/glad/include -I../libs/imgui -I../libs/stb -I../libs/spirv-cross -I../libs/assimp/include -I../libs/generated
DEFINES := -DGLFW_INCLUDE_NONE
LIBS := -l:libglad_gl.a -lglfw -l:libimgui_gl_glfw.a
LIBDIRS := -L../libs/generated
MISCOPTS := -std=c++20 -c
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

OPTS := $(INCLUDES) $(DEFINES) $(LIBDIRS) $(MISCOPTS) $(CFGFLAGS)

OBJECTS := $(OBJDIR)/Window.o $(OBJDIR)/Shader.o

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
	@$(cpp) -shared $(OBJECTS) $(CFGFLAGS) $(LIBDIRS) $(LIBS) $(subst -c,,$(MISCOPTS)) -o $(BIN)

build: dirs prebuild $(OBJECTS) link
	@:

# Files
###########

$(OBJDIR)/Window.o: $(SRCDIR)/Window.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<
$(OBJDIR)/Shader.o: $(SRCDIR)/Shader.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<