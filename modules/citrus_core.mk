SHELLTYPE := posix
ifeq (.exe,$(findstring .exe,$(ComSpec)))
  SHELLTYPE := msdos
endif

ifndef target
	target=$(shell $(cpp) -dumpmachine)
endif

.PHONY: default build link prebuild dirs

INCLUDES := -I../CitrusEngine/src -I../libs/spdlog/include -I../libs/glm -I../libs/boost/include -I../libs -I../libs/imgui -I../libs/stb -I../libs/assimp/include -I../assimp_generated
DEFINES :=
LIBS := -limgui_core -lstb -lassimp
LIBDIRS := -L/usr/lib64 -L../Build/modules/imgui_core/$(target)/$(config)/bin
MISCOPTS := -std=gnu++20 -c
CFGFLAGS :=

SRCDIR := ../CitrusEngine/src
OBJDIR := ../Build/modules/citrus_core/$(target)/$(config)/obj
BINDIR := ../Build/modules/citrus_core/$(target)/$(config)/bin
BIN := $(BINDIR)/libcitrus_core.a

ifeq ($(config),debug)
	CFGFLAGS += -g -DCE_ENV_DEBUG
else ifeq ($(config),release)
	CFGFLAGS += -O2 -DCE_ENV_RELEASE
endif

OPTS := $(INCLUDES) $(DEFINES) $(LIBDIRS) $(LIBS) $(MISCOPTS) $(CFGFLAGS)

OBJECTS := $(OBJDIR)/Entrypoint.o $(OBJDIR)/Log.o $(OBJDIR)/Assert.o $(OBJDIR)/CitrusClient.o $(OBJDIR)/EventManager.o $(OBJDIR)/OrthographicCamera.o $(OBJDIR)/PerspectiveCamera.o $(OBJDIR)/Renderer.o $(OBJDIR)/ImGuiWrapper.o $(OBJDIR)/Input.o $(OBJDIR)/Utilities.o $(OBJDIR)/Model.o

default:
	@echo "Citrus Engine citrus_core Module Builder"
	@echo "========================================="
	@echo ""
	@echo "Do not run directly. This should only be invoked by the modulebuild.mk Makefile."

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

prebuild:
	@echo "Module 'citrus_core' build: Compile Objects (1/2)"

link:
	@echo ""
	@echo "Module 'citrus_core' build: Link Library (2/2)"
	@echo "Linking '$(subst ../,,$(BIN))'..."
ifeq (posix,$(SHELLTYPE))
	@mkdir -p $(BINDIR)/tmp
else
	@mkdir $(subst /,\\,$(BINDIR)/tmp)
endif
	@$(AR) -x ../Build/modules/imgui_core/$(target)/$(config)/bin/libimgui_core.a --output $(BINDIR)/tmp
	@$(AR) -x ../Build/modules/stb/$(target)/$(config)/bin/libstb.a --output $(BINDIR)/tmp
	@$(AR) -x ../Build/modules/assimp/$(target)/$(config)/bin/libassimp.a --output $(BINDIR)/tmp
	@$(AR) -rcs $(BIN) $(OBJECTS) $(BINDIR)/tmp/*
ifeq (posix,$(SHELLTYPE))
	@rm -rf $(BINDIR)/tmp
else
	@if exist $(subst /,\\,$(BINDIR)/tmp) rmdir /s /q $(subst /,\\,$(BINDIR)/tmp)
endif

build: dirs prebuild $(OBJECTS) link
	@:

# Files
###########

$(OBJDIR)/Entrypoint.o: $(SRCDIR)/Core/Entrypoint.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/Log.o: $(SRCDIR)/Core/Log.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/Assert.o: $(SRCDIR)/Core/Assert.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/CitrusClient.o: $(SRCDIR)/Core/CitrusClient.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/EventManager.o: $(SRCDIR)/Events/EventManager.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/OrthographicCamera.o: $(SRCDIR)/Graphics/Cameras/OrthographicCamera.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/PerspectiveCamera.o: $(SRCDIR)/Graphics/Cameras/PerspectiveCamera.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/Renderer.o: $(SRCDIR)/Graphics/Renderer.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/ImGuiWrapper.o: $(SRCDIR)/ImGui/ImGuiWrapper.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/Input.o: $(SRCDIR)/Utilities/Input.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/Utilities.o: $(SRCDIR)/Utilities/Utilities.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<
$(OBJDIR)/Model.o: $(SRCDIR)/Models/Model.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<
