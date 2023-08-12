SHELLTYPE := posix
ifeq (.exe,$(findstring .exe,$(ComSpec)))
  SHELLTYPE := msdos
endif

ifndef target
	target=$(shell $(cpp) -dumpmachine)
endif

.PHONY: default build link prebuild dirs

INCLUDES := -I../libs/imgui
DEFINES :=
LIBS :=
LIBDIRS := -L/usr/lib64
MISCOPTS := -std=gnu++20 -c
CFGFLAGS :=

SRCDIR := ../libs/imgui
OBJDIR := ../Build/modules/imgui_core/$(target)/$(config)/obj
BINDIR := ../Build/modules/imgui_core/$(target)/$(config)/bin
BIN := $(BINDIR)/libimgui_core.a

ifeq ($(config),debug)
	CFGFLAGS += -g
else ifeq ($(config),release)
	CFGFLAGS += -O2
endif

OPTS := $(INCLUDES) $(DEFINES) $(LIBDIRS) $(LIBS) $(MISCOPTS) $(CFGFLAGS)

OBJECTS := $(OBJDIR)/imgui.o $(OBJDIR)/imgui_widgets.o $(OBJDIR)/imgui_draw.o $(OBJDIR)/imgui_demo.o $(OBJDIR)/imgui_tables.o

default:
	@echo "Citrus Engine imgui_core Module Builder"
	@echo "========================================="
	@echo ""
	@echo "Do not run directly. This should only be invoked by the modulebuild.mk Makefile."

dirs:
ifeq (posix,$(SHELLTYPE))
	$(SILENT) mkdir -p $(OBJDIR)
else
	$(SILENT) mkdir $(subst /,\\,$(OBJDIR))
endif
ifeq (posix,$(SHELLTYPE))
	$(SILENT) mkdir -p $(BINDIR)
else
	$(SILENT) mkdir $(subst /,\\,$(BINDIR))
endif

prebuild:
	@echo "Module 'imgui_core' build: Compile Objects (1/2)"

link:
	@echo ""
	@echo "Module 'imgui_core' build: Link Library (2/2)"
	@echo "Linking '$(BIN)'..."
	@$(AR) -rcs $(BIN) $(OBJECTS)

build: dirs prebuild $(OBJECTS) link
	@:

# Files
###########

$(OBJDIR)/imgui.o: $(SRCDIR)/imgui.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/imgui_draw.o: $(SRCDIR)/imgui_draw.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/imgui_demo.o: $(SRCDIR)/imgui_demo.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/imgui_widgets.o: $(SRCDIR)/imgui_widgets.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/imgui_tables.o: $(SRCDIR)/imgui_tables.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" "$<"