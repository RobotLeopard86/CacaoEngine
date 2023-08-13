SHELLTYPE := posix
ifeq (.exe,$(findstring .exe,$(ComSpec)))
  SHELLTYPE := msdos
endif

ifndef target
	target=$(shell $(cpp) -dumpmachine)
endif

.PHONY: default build link prebuild dirs

INCLUDES := -I../libs/imgui -I../libs/imgui/backends
DEFINES :=
LIBS :=
LIBDIRS := -L/usr/lib64
MISCOPTS := -std=gnu++20 -c
CFGFLAGS :=

SRCDIR := ../libs/imgui
OBJDIR := ../Build/modules/imgui_gl3/$(target)/$(config)/obj
BINDIR := ../Build/modules/imgui_gl3/$(target)/$(config)/bin
BIN := $(BINDIR)/libimgui_gl3.a

ifeq ($(config),debug)
	CFGFLAGS += -g
else ifeq ($(config),release)
	CFGFLAGS += -O2
endif

OPTS := $(INCLUDES) $(DEFINES) $(LIBDIRS) $(LIBS) $(MISCOPTS) $(CFGFLAGS)

OBJECTS := $(OBJDIR)/imgui_impl_opengl3.o

default:
	@echo "Citrus Engine imgui_gl3 Module Builder"
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
	@echo "Module 'imgui_gl3' build: Compile Objects (1/2)"

link:
	@echo ""
	@echo "Module 'imgui_gl3' build: Link Library (2/2)"
	@echo "Linking '$(subst ../,,$(BIN))'..."
	@$(AR) -rcs $(BIN) $(OBJECTS)

build: dirs prebuild $(OBJECTS) link
	@:

# Files
###########

$(OBJDIR)/imgui_impl_opengl3.o: $(SRCDIR)/backends/imgui_impl_opengl3.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<
