SHELLTYPE := posix
ifeq (.exe,$(findstring .exe,$(ComSpec)))
  SHELLTYPE := msdos
endif

ifndef target
	target=$(shell $(cpp) -dumpmachine)
endif

.PHONY: default build link prebuild dirs

INCLUDES := -I../CitrusEngine/include/CitrusEngine -I../libs/glm -I../libs/spdlog/include -I../libs/glfw/include -I../libs/glad/include -I../libs/boost/include -I../libs -I../libs/imgui -I../libs/stb
DEFINES := -DGLFW_INCLUDE_NONE -DCE_GLFW_API=GLFW_OPENGL_API
LIBS := -lglfw_x -limgui_glfw -limgui_gl3 -lglad_gl3 -lGL
LIBDIRS := -L/usr/lib64 -L../Build/modules/glfw_x/$(target)/$(config)/bin -L../Build/modules/imgui_glfw/$(target)/$(config)/bin -L../Build/modules/imgui_gl3/$(target)/$(config)/bin -L../Build/modules/glad_gl3/$(target)/$(config)/bin
MISCOPTS := -std=gnu++20 -c
CFGFLAGS :=

SRCDIR := ../CitrusEngine/src
OBJDIR := ../Build/modules/citrus_backend_glfwx_gl3/$(target)/$(config)/obj
BINDIR := ../Build/modules/citrus_backend_glfwx_gl3/$(target)/$(config)/bin
BIN := $(BINDIR)/libcitrus_backend_glfwx_gl3.a

ifeq ($(config),debug)
	CFGFLAGS += -g -DCE_ENV_DEBUG
else ifeq ($(config),release)
	CFGFLAGS += -O2 -DCE_ENV_RELEASE
endif

OPTS := $(INCLUDES) $(DEFINES) $(LIBDIRS) $(LIBS) $(MISCOPTS) $(CFGFLAGS)

OBJECTS := $(OBJDIR)/glfwx_gl3_Backend.o $(OBJDIR)/glfwx_gl3_ImGui.o $(OBJDIR)/GLFWBackendComponent.o $(OBJDIR)/GLFWUtilities.o $(OBJDIR)/GLFWWindow.o $(OBJDIR)/OpenGLMesh.o $(OBJDIR)/OpenGLRenderer.o $(OBJDIR)/OpenGLShader.o

default:
	@echo "Citrus Engine citrus_backend_glfwx_gl3 Module Builder"
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
	@echo "Module 'citrus_backend_glfwx_gl3' build: Compile Objects (1/2)"

link:
	@echo ""
	@echo "Module 'citrus_backend_glfwx_gl3' build: Link Library (2/2)"
	@echo "Linking '$(subst ../,,$(BIN))'..."
ifeq (posix,$(SHELLTYPE))
	@mkdir -p $(BINDIR)/tmp
else
	@mkdir $(subst /,\\,$(BINDIR)/tmp)
endif
	@$(AR) -x ../Build/modules/imgui_glfw/$(target)/$(config)/bin/libimgui_glfw.a --output $(BINDIR)/tmp
	@$(AR) -x ../Build/modules/imgui_gl3/$(target)/$(config)/bin/libimgui_gl3.a --output $(BINDIR)/tmp
	@$(AR) -x ../Build/modules/glad_gl3/$(target)/$(config)/bin/libglad_gl3.a --output $(BINDIR)/tmp
	@$(AR) -x ../Build/modules/glfw_x/$(target)/$(config)/bin/libglfw_x.a --output $(BINDIR)/tmp
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

$(OBJDIR)/glfwx_gl3_Backend.o: $(SRCDIR)/Native/Backends/glfwx_gl3/glfwx_gl3_Backend.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/glfwx_gl3_ImGui.o: $(SRCDIR)/Native/Backends/glfwx_gl3/glfwx_gl3_ImGui.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/GLFWBackendComponent.o: $(SRCDIR)/Native/Common/GLFW/GLFWBackendComponent.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/GLFWUtilities.o: $(SRCDIR)/Native/Common/GLFW/GLFWUtilities.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/GLFWWindow.o: $(SRCDIR)/Native/Common/GLFW/GLFWWindow.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/OpenGLMesh.o: $(SRCDIR)/Native/Common/OpenGL/OpenGLMesh.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/OpenGLRenderer.o: $(SRCDIR)/Native/Common/OpenGL/OpenGLRenderer.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/OpenGLShader.o: $(SRCDIR)/Native/Common/OpenGL/OpenGLShader.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<
