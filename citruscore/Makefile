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

INCLUDES := -Iinclude -I../libs/spdlog/include -I../libs/glm -I../libs/bsthreadpool -I../libs/spirv-cross -I../libs -I../libs/imgui -I../libs/stb -I../libs/assimp/include -I../libs/generated
DEFINES :=
LIBS :=
LIBDIRS :=
MISCOPTS := -std=c++20 -c
CFGFLAGS :=

SRCDIR := src
OBJDIR := build/$(target)/$(config)/obj
BINDIR := build/$(target)/$(config)/bin
BIN := $(BINDIR)/libcitruscore.a

ifeq ($(config),debug)
	CFGFLAGS += -g -DCE_ENV_DEBUG
else ifeq ($(config),release)
	CFGFLAGS += -O2 -s -DCE_ENV_RELEASE
endif

OPTS := $(INCLUDES) $(DEFINES) $(LIBDIRS) $(MISCOPTS) $(CFGFLAGS)

OBJECTS := $(OBJDIR)/Startup.o $(OBJDIR)/Log.o $(OBJDIR)/Assert.o $(OBJDIR)/Engine.o $(OBJDIR)/EventManager.o $(OBJDIR)/Input.o $(OBJDIR)/Model.o $(OBJDIR)/Transform.o $(OBJDIR)/PerspectiveCamera.o $(OBJDIR)/WorldManager.o $(OBJDIR)/LogicPhase.o $(OBJDIR)/ProcessPhase.o $(OBJDIR)/RenderPhase.o

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
	@echo "Citrus Core build: Compile Objects (1/2)"

link:
	@echo ""
	@echo "Citrus Core build: Link Library (2/2)"
	@echo "Linking '$(BIN)'..."
	@ar -rcs $(BIN) $(OBJECTS)

build: dirs prebuild $(OBJECTS) link
	@:

# Files
###########

$(OBJDIR)/Startup.o: $(SRCDIR)/Core/Startup.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/Log.o: $(SRCDIR)/Core/Log.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/Assert.o: $(SRCDIR)/Core/Assert.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/Engine.o: $(SRCDIR)/Core/Engine.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/EventManager.o: $(SRCDIR)/Events/EventManager.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/Input.o: $(SRCDIR)/Utilities/Input.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/Transform.o: $(SRCDIR)/3D/Transform.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/Model.o: $(SRCDIR)/3D/Model.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/PerspectiveCamera.o: $(SRCDIR)/Cameras/PerspectiveCamera.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/WorldManager.o: $(SRCDIR)/World/WorldManager.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/LogicPhase.o: $(SRCDIR)/Pipeline/LogicPhase.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/ProcessPhase.o: $(SRCDIR)/Pipeline/ProcessPhase.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/RenderPhase.o: $(SRCDIR)/Pipeline/RenderPhase.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<