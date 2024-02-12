SHELLTYPE := posix
ifeq (.exe,$(findstring .exe,$(ComSpec)))
  SHELLTYPE := msdos
endif

ifndef target
	target != $(cpp) -dumpmachine
endif

.PHONY: default build link prebuild dirs

INCLUDES := -I../CacaoEngine/include/ -I../libs/spdlog/include -I../libs/glm -I../libs -I../libs/imgui
DEFINES :=
LIBS := -lcacao_core -l$(backend)
LIBDIRS := -L/usr/lib64 -L../Build/modules/cacao_core/$(target)/$(config)/bin -L../Build/modules/$(backend)/$(target)/$(config)/bin
MISCOPTS := -std=gnu++20 -c
CFGFLAGS :=

SRCDIR := ../CacaoPlayground/src
OBJDIR := ../Build/playground/$(target)/$(config)/$(backend)/obj
BINDIR := ../Build/playground/$(target)/$(config)/$(backend)/bin
BIN := $(BINDIR)/CacaoPlayground

ifeq ($(config),debug)
	CFGFLAGS += -g -DCE_ENV_DEBUG
else ifeq ($(config),release)
	CFGFLAGS += -O2 -DCE_ENV_RELEASE
endif

OPTS := $(INCLUDES) $(DEFINES) $(LIBDIRS) $(LIBS) $(MISCOPTS) $(CFGFLAGS)

OBJECTS := $(OBJDIR)/Playground.o

default:
	@echo "Cacao Engine Playground Builder"
	@echo "========================================="
	@echo ""
	@echo "Do not run directly. This should only be invoked by the top-level Makefile."

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
	@echo "Cacao Playground build: Compile Objects (1/2)"

link:
	@echo ""
	@echo "Cacao Playground build: Link Executable (2/2)"
	@echo "Linking '$(subst ../,,$(BIN))'..."
	@$(cpp) $(subst -c,,$(OPTS)) -o "$(BIN)" $(OBJECTS)

build: dirs prebuild $(OBJECTS) link
	@:

# Files
###########

$(OBJDIR)/Playground.o: $(SRCDIR)/Playground.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<