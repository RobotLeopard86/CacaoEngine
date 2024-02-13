SHELLTYPE := posix
ifeq (.exe,$(findstring .exe,$(ComSpec)))
  SHELLTYPE := msdos
endif

ifndef c
	c=clang
endif

ifndef cpp
	cpp=clang++
endif

ifndef config
  config=debug
endif

.PHONY: build link prebuild dirs clean

INCLUDES := -Iinclude -I../cacao/include -I../libs/spdlog/include -I../libs/glm -I../libs -I../libs/glad/include -I../libs/imgui -I../libs/stb -I../libs/spirv-cross -I../libs/assimp/include -I../libs/generated
DEFINES := -DGLFW_INCLUDE_NONE
LIBS := -lglad_gl -lglfw -limgui_gl_glfw -lspirv-cross-c -lspirv-cross-core -lspirv-cross-cpp -lspirv-cross-glsl -lspirv-cross-reflect -lspirv-cross-util -lspirv-cross-hlsl -lspirv-cross-msl
LIBDIRS := -L../libs/generated
MISCOPTS := -std=c++20 -c -fPIC
CFGFLAGS :=

SRCDIR := src
OBJDIR := build/$(config)/obj
BINDIR := build/$(config)/bin
BIN := $(BINDIR)/libcacaoglglfw.so

ifeq ($(config),debug)
	CFGFLAGS += -g -DCE_ENV_DEBUG
else ifeq ($(config),release)
	CFGFLAGS += -O2 -s -DCE_ENV_RELEASE
endif

OPTS := $(INCLUDES) $(DEFINES) $(MISCOPTS) $(CFGFLAGS)

OBJECTS := $(OBJDIR)/Window.o $(OBJDIR)/Shader.o $(OBJDIR)/RenderPhaseImpl.o

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
	@echo "Cacao GLFW+OpenGL Backend build: Compile Objects (1/2)"

link:
	@echo ""
	@echo "Cacao GLFW+OpenGL Backend build: Link Library (2/2)"
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
$(OBJDIR)/RenderPhaseImpl.o: $(SRCDIR)/RenderPhaseImpl.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<