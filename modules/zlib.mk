SHELLTYPE := posix
ifeq (.exe,$(findstring .exe,$(ComSpec)))
  SHELLTYPE := msdos
endif

ifndef target
	target=$(shell $(cpp) -dumpmachine)
endif

.PHONY: default build link prebuild dirs

INCLUDES := -I../libs/assimp/contrib/zlib
DEFINES :=
LIBS :=
LIBDIRS := -L/usr/lib64
MISCOPTS := -std=gnu17 -c
CFGFLAGS :=

SRCDIR := ../libs/assimp/contrib/zlib
OBJDIR := ../Build/modules/zlib/$(target)/$(config)/obj
BINDIR := ../Build/modules/zlib/$(target)/$(config)/bin
BIN := $(BINDIR)/libzlib.a

ifeq ($(config),debug)
	CFGFLAGS += -g
else ifeq ($(config),release)
	CFGFLAGS += -O2
endif

OPTS := $(INCLUDES) $(DEFINES) $(LIBDIRS) $(LIBS) $(MISCOPTS) $(CFGFLAGS)

OBJECTS := $(OBJDIR)/crc32.o $(OBJDIR)/gzlib.o $(OBJDIR)/gzclose.o $(OBJDIR)/infback.o $(OBJDIR)/deflate.o $(OBJDIR)/compress.o $(OBJDIR)/inftrees.o $(OBJDIR)/inflate.o $(OBJDIR)/gzread.o $(OBJDIR)/inffast.o $(OBJDIR)/gzwrite.o $(OBJDIR)/adler32.o $(OBJDIR)/trees.o $(OBJDIR)/uncompr.o $(OBJDIR)/zutil.o

default:
	@echo "Citrus Engine zlib Module Builder"
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
	@echo "Module 'zlib' build: Compile Objects (1/2)"

link:
	@echo ""
	@echo "Module 'zlib' build: Link Library (2/2)"
	@echo "Linking '$(subst ../,,$(BIN))'..."
	@$(AR) -rcs $(BIN) $(OBJECTS)

build: dirs prebuild $(OBJECTS) link
	@:

# Files
###########

$(OBJDIR)/crc32.o: $(SRCDIR)/crc32.c
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(c) $(OPTS) -o "$@" $<

$(OBJDIR)/gzlib.o: $(SRCDIR)/gzlib.c
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(c) $(OPTS) -o "$@" $<

$(OBJDIR)/gzclose.o: $(SRCDIR)/gzclose.c
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(c) $(OPTS) -o "$@" $<

$(OBJDIR)/infback.o: $(SRCDIR)/infback.c
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(c) $(OPTS) -o "$@" $<

$(OBJDIR)/deflate.o: $(SRCDIR)/deflate.c
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(c) $(OPTS) -o "$@" $<

$(OBJDIR)/compress.o: $(SRCDIR)/compress.c
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(c) $(OPTS) -o "$@" $<

$(OBJDIR)/inftrees.o: $(SRCDIR)/inftrees.c
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(c) $(OPTS) -o "$@" $<

$(OBJDIR)/inflate.o: $(SRCDIR)/inflate.c
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(c) $(OPTS) -o "$@" $<

$(OBJDIR)/gzread.o: $(SRCDIR)/gzread.c
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(c) $(OPTS) -o "$@" $<

$(OBJDIR)/inffast.o: $(SRCDIR)/inffast.c
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(c) $(OPTS) -o "$@" $<

$(OBJDIR)/gzwrite.o: $(SRCDIR)/gzwrite.c
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(c) $(OPTS) -o "$@" $<

$(OBJDIR)/adler32.o: $(SRCDIR)/adler32.c
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(c) $(OPTS) -o "$@" $<

$(OBJDIR)/trees.o: $(SRCDIR)/trees.c
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(c) $(OPTS) -o "$@" $<

$(OBJDIR)/uncompr.o: $(SRCDIR)/uncompr.c
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(c) $(OPTS) -o "$@" $<

$(OBJDIR)/zutil.o: $(SRCDIR)/zutil.c
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(c) $(OPTS) -o "$@" $<
