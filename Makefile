ifndef config
  config=debug
endif

ifndef module
  module=none
endif

CONFIGS := debug release
MODULES := imgui_core glad_gl3 glfw_x imgui_gl3 imgui_glfw citrus_core citrus_backend_glfwx_gl3 stb
BACKENDS := glfwx_gl3

ifndef c
	c=zig cc
endif

ifndef cpp
	cpp=zig c++
endif

SHELLTYPE := posix
ifeq (.exe,$(findstring .exe,$(ComSpec)))
  SHELLTYPE := msdos
endif

ifndef target
	target != $(cpp) -dumpmachine
endif

ifeq (posix,$(SHELLTYPE))
	builtmods != if [ -d Build/modules ]; then ls Build/modules; fi
else
	builtmods= != if exist $(subst /,\\,Build/modules) dir $(subst /,\\,Build/modules)
endif

.PHONY: help build-module build-playground clean-module clean-all run-playground

# Note: commented out modules will eventually be supported but currently are not.

help:
	@echo "Citrus Engine"
	@echo "==================="
	@echo ""
	@echo "Building modules: make build-module config=[config (default: debug)] c=[alternate C compiler (default: zig cc)] cpp=[alternate C++ compiler (default: zig c++)] module=<module>"
	@echo "Building playground: make build-playground backend=<backend module>"
	@echo "Cleaning modules: make clean-module module=<module>"
	@echo "Cleaning everything: make clean-all"
	@echo "Running playground: make playground backend=<backend>"
	@echo "Angle brackets (<>) denote required parameter. Square brackets ([]) denote optional parameter"
	@echo ""
	@echo "Valid configs:"
	@echo "  debug"
	@echo "  release"
	@echo ""
	@echo "Valid modules:"
	@echo "  stb (STB utilities library)"
	@echo "  imgui_core (ImGui library without platform backends)"
	@echo "  glad_gl3 (Glad OpenGL 3 library)"
# @echo "  glad_vk (Glad Vulkan library)"
# @echo "  glfw_osx (GLFW library for macOS)"
# @echo "  glfw_win (GLFW library for Windows)"
# @echo "  glfw_wl (GLFW library for Wayland on Linux)"
	@echo "  glfw_x (GLFW library for X11 on Linux)"
	@echo "  imgui_gl3 (ImGui backend library for OpenGL 3) (dependencies: imgui_core, glad_gl3)"
# @echo "  imgui_dx12 (ImGui backend library for DirectX 12) (dependencies: imgui_core)"
# @echo "  imgui_metal (ImGui backend library for Metal) (dependencies: imgui_core)"
# @echo "  imgui_vk (ImGui backend library for Vulkan) (dependencies: imgui_core, glad_vk)"
	@echo "  imgui_glfw (ImGui backend library for GLFW) (dependencies: imgui_core)"
# @echo "  imgui_win (ImGui backend library for Windows)"
	@echo "  citrus_core (Core Citrus Engine library) (dependencies: imgui_core)"
	@echo "Valid backend modules:"
	@echo "  citrus_backend_glfwx_gl3 (Backend using GLFW (X11) and OpenGL 3) (dependencies: citrus_core, imgui_glfw, imgui_gl3, glfw_x)"


clean-all:
	@echo "Cleaning all modules..."
ifeq (posix,$(SHELLTYPE))
	@rm -rf Build
else
	@if exist $(subst /,\\,Build) rmdir /s /q $(subst /,\\,Build)
endif 
	@echo "Done!"

clean-module:
ifeq (none,$(module))
	$(error "You must specify the module parameter")
endif
ifeq ($(filter $(module),$(MODULES)),)
	$(error '$(module)' is not a valid module! Please refer to 'make help' for help)
endif
	@echo "Cleaning module $(module)..."
ifeq (posix,$(SHELLTYPE))
	@rm -rf Build/modules/$(module)
else
	@if exist $(subst /,\\,Build) rmdir /s /q $(subst /,\\,Build/modules/$(module))
endif 
	@echo "Done!"

build-playground:
ifeq ($(filter $(config),$(CONFIGS)),)
  	$(error '$(config)' is not a valid configuration! Please refer to 'make help' for help)
endif
ifeq (none,$(backend))
	$(error "You must specify the backend parameter")
endif
ifeq ($(filter $(backend),$(BACKENDS)),)
	$(error '$(backend)' is not a valid backend! Please refer to 'make help' for help)
endif
ifeq ($(filter $(builtmods),citrus_backend_$(backend)),)
	@echo "Backend '$(backend)' has not been built. Building backend and dependencies first..."
	@${MAKE} --no-print-directory build-module config=$(config) c='$(c)' cpp='$(cpp)' module=citrus_backend_$(backend)
endif
	@echo "Building playground using backend '$(backend)'..."
	@${MAKE} --no-print-directory -C CitrusPlayground -f Makefile build config=$(config) c='$(c)' cpp='$(cpp)' backend=citrus_backend_$(backend)
	@echo "Done!"

run-playground:
ifeq (none,$(backend))
	$(error "You must specify the backend parameter")
endif
ifeq ($(filter $(backend),$(BACKENDS)),)
	$(error '$(backend)' is not a valid backend! Please refer to 'make help' for help)
endif
	@Build/playground/$(target)/$(config)/citrus_backend_$(backend)/bin/CitrusPlayground

build-module:
ifeq ($(filter $(config),$(CONFIGS)),)
  	$(error '$(config)' is not a valid configuration! Please refer to 'make help' for help)
endif
ifeq (none,$(module))
	$(error "You must specify the module parameter")
endif
ifeq ($(filter $(module),$(MODULES)),)
	$(error '$(module)' is not a valid module! Please refer to 'make help' for help)
endif
	@${MAKE} --no-print-directory -C modules -f modulebuild.mk $(module) config=$(config) c='$(c)' cpp='$(cpp)'