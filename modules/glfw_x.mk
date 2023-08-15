SHELLTYPE := posix
ifeq (.exe,$(findstring .exe,$(ComSpec)))
  SHELLTYPE := msdos
endif

ifndef target
	target=$(shell $(cpp) -dumpmachine)
endif

.PHONY: default build link prebuild dirs

INCLUDES := -I../libs/glfw/include -I../libs/glfw/src
DEFINES := -D_GLFW_X11
LIBS :=
LIBDIRS := -L/usr/lib64
MISCOPTS := -std=gnu17 -c
CFGFLAGS :=

SRCDIR := ../libs/glfw/src
OBJDIR := ../Build/modules/glfw_x/$(target)/$(config)/obj
BINDIR := ../Build/modules/glfw_x/$(target)/$(config)/bin
BIN := $(BINDIR)/libglfw_x.a

ifeq ($(config),debug)
	CFGFLAGS += -g
else ifeq ($(config),release)
	CFGFLAGS += -O2
endif

OPTS := $(INCLUDES) $(DEFINES) $(LIBDIRS) $(LIBS) $(MISCOPTS) $(CFGFLAGS)

OBJECTS := $(OBJDIR)/context.o $(OBJDIR)/init.o $(OBJDIR)/input.o $(OBJDIR)/monitor.o $(OBJDIR)/platform.o $(OBJDIR)/vulkan.o $(OBJDIR)/window.o\
$(OBJDIR)/egl_context.o $(OBJDIR)/osmesa_context.o $(OBJDIR)/null_init.o $(OBJDIR)/null_monitor.o $(OBJDIR)/null_window.o $(OBJDIR)/null_joystick.o\
$(OBJDIR)/posix_module.o $(OBJDIR)/posix_time.o $(OBJDIR)/posix_thread.o $(OBJDIR)/posix_poll.o $(OBJDIR)/x11_init.o $(OBJDIR)/x11_monitor.o $(OBJDIR)/x11_window.o\
$(OBJDIR)/xkb_unicode.o $(OBJDIR)/glx_context.o $(OBJDIR)/linux_joystick.o

default:
	@echo "Citrus Engine glfw_x Module Builder"
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
	@echo "Module 'glfw_x' build: Compile Objects (1/2)"

link:
	@echo ""
	@echo "Module 'glfw_x' build: Link Library (2/2)"
	@echo "Linking '$(subst ../,,$(BIN))'..."
	@$(AR) -rcs $(BIN) $(OBJECTS)

build: dirs prebuild $(OBJECTS) link
	@:

# Files
###########

$(OBJDIR)/context.o: $(SRCDIR)/context.c
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(c) $(OPTS) -o "$@" $<

$(OBJDIR)/init.o: $(SRCDIR)/init.c
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(c) $(OPTS) -o "$@" $<

$(OBJDIR)/input.o: $(SRCDIR)/input.c
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(c) $(OPTS) -o "$@" $<

$(OBJDIR)/platform.o: $(SRCDIR)/platform.c
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(c) $(OPTS) -o "$@" $<

$(OBJDIR)/vulkan.o: $(SRCDIR)/window.c
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(c) $(OPTS) -o "$@" $<

$(OBJDIR)/monitor.o: $(SRCDIR)/monitor.c
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(c) $(OPTS) -o "$@" $<

$(OBJDIR)/window.o: $(SRCDIR)/window.c
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(c) $(OPTS) -o "$@" $<

$(OBJDIR)/egl_context.o: $(SRCDIR)/egl_context.c
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(c) $(OPTS) -o "$@" $<

$(OBJDIR)/osmesa_context.o: $(SRCDIR)/osmesa_context.c
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(c) $(OPTS) -o "$@" $<

$(OBJDIR)/null_init.o: $(SRCDIR)/null_init.c
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(c) $(OPTS) -o "$@" $<

$(OBJDIR)/null_monitor.o: $(SRCDIR)/null_monitor.c
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(c) $(OPTS) -o "$@" $<

$(OBJDIR)/null_window.o: $(SRCDIR)/null_window.c
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(c) $(OPTS) -o "$@" $<

$(OBJDIR)/null_joystick.o: $(SRCDIR)/null_joystick.c
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(c) $(OPTS) -o "$@" $<

$(OBJDIR)/posix_module.o: $(SRCDIR)/posix_module.c
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(c) $(OPTS) -o "$@" $<

$(OBJDIR)/posix_time.o: $(SRCDIR)/posix_time.c
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(c) $(OPTS) -o "$@" $<

$(OBJDIR)/posix_thread.o: $(SRCDIR)/posix_thread.c
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(c) $(OPTS) -o "$@" $<

$(OBJDIR)/posix_poll.o: $(SRCDIR)/posix_poll.c
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(c) $(OPTS) -o "$@" $<

$(OBJDIR)/x11_init.o: $(SRCDIR)/x11_init.c
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(c) $(OPTS) -o "$@" $<

$(OBJDIR)/x11_monitor.o: $(SRCDIR)/x11_monitor.c
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(c) $(OPTS) -o "$@" $<

$(OBJDIR)/x11_window.o: $(SRCDIR)/x11_window.c
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(c) $(OPTS) -o "$@" $<

$(OBJDIR)/xkb_unicode.o: $(SRCDIR)/xkb_unicode.c
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(c) $(OPTS) -o "$@" $<

$(OBJDIR)/glx_context.o: $(SRCDIR)/glx_context.c
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(c) $(OPTS) -o "$@" $<

$(OBJDIR)/linux_joystick.o: $(SRCDIR)/linux_joystick.c
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(c) $(OPTS) -o "$@" $<