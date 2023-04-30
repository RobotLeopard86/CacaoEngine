ifndef config
  config=debug
endif

ifeq ($(config),debug)
  GLFW_config = debug
  ImGui_config = debug
  Glad_config = debug
  CitrusEngine_config = debug
  CitrusPlayground_config = debug

else ifeq ($(config),release)
  GLFW_config = release
  ImGui_config = release
  Glad_config = release
  CitrusEngine_config = release
  CitrusPlayground_config = release

else
  $(error "Invalid configuration $(config)")
endif

PROJECTS := GLFW ImGui Glad CitrusEngine CitrusPlayground

.PHONY: all clean help $(PROJECTS) 

SHELLTYPE := posix
ifeq (.exe,$(findstring .exe,$(ComSpec)))
	SHELLTYPE := msdos
endif

all: $(PROJECTS)

GLFW:
ifneq (,$(GLFW_config))
	@echo "Building GLFW - $(GLFW_config)..."
	@${MAKE} CXX="zig c++" CC="zig cc" --no-print-directory -C libs/glfw -f Makefile config=$(GLFW_config)
	@echo "Done building GLFW - $(GLFW_config)."
endif

ImGui:
ifneq (,$(ImGui_config))
	@echo "Building ImGui - $(ImGui_config)..."
	@${MAKE} CXX="zig c++" CC="zig cc" --no-print-directory -C libs/imgui -f Makefile config=$(ImGui_config)
	@echo "Done building ImGui - $(ImGui_config)."
endif

Glad:
ifneq (,$(Glad_config))
	@echo "Building Glad - $(Glad_config)..."
	@${MAKE} CXX="zig c++" CC="zig cc" --no-print-directory -C libs/glad -f Makefile config=$(Glad_config)
	@echo "Done building Glad - $(Glad_config)."
endif

CitrusEngine: deps
ifneq (,$(CitrusEngine_config))
	@echo "Building CitrusEngine - $(CitrusEngine_config)..."
	@${MAKE} CXX="zig c++" CC="zig cc" renderapi=$(renderapi) --no-print-directory -C CitrusEngine -f Makefile config=$(CitrusEngine_config)
	@echo "Done building CitrusEngine - $(CitrusEngine_config)."
endif

CitrusPlayground: CitrusEngine
ifneq (,$(CitrusPlayground_config))
	@echo "Building CitrusPlayground - $(CitrusPlayground_config)..."
	@${MAKE} CXX="zig c++" CC="zig cc" --no-print-directory -C CitrusPlayground -f Makefile config=$(CitrusPlayground_config)
	@echo "Done building CitrusPlayground - $(CitrusPlayground_config)."
endif

clean:
	@echo "Cleaning all build files..."
	@${MAKE} CXX="zig c++" CC="zig cc" --no-print-directory -C libs/glfw -f Makefile clean
	@${MAKE} CXX="zig c++" CC="zig cc" --no-print-directory -C libs/imgui -f Makefile clean
	@${MAKE} CXX="zig c++" CC="zig cc" --no-print-directory -C libs/glad -f Makefile clean
	@${MAKE} CXX="zig c++" CC="zig cc" --no-print-directory -C CitrusEngine -f Makefile clean
	@${MAKE} CXX="zig c++" CC="zig cc" --no-print-directory -C CitrusPlayground -f Makefile clean
ifeq (posix,$(SHELLTYPE))
	@rm -rf Build
else
	@if exist $(subst /,\\,Build) rmdir /s /q $(subst /,\\,Build)
endif
	@echo "Cleaning done."

deps: GLFW Glad ImGui
	@echo "Done building CitrusEngine dependencies."

run:
	@echo "Running latest Debug build..."
	@Build/CitrusPlayground/Debug/x86_64-linux/Binaries/CitrusPlayground
	@echo "Done."

run-release:
	@echo "Running latest Release build..."
	@Build/CitrusPlayground/Release/x86_64-linux/Binaries/CitrusPlayground
	@echo "Done."

build-run:
	@echo "Building all..."
	@${MAKE} --no-print-directory all
	@echo "Done building."
	@${MAKE} --no-print-directory run

test: CitrusEngine
	@echo "Building and running tests..."
	@${MAKE} CXX="zig c++" CC="zig cc" --no-print-directory -C CitrusEngine -f Makefile config=$(CitrusEngine_config) test
	@echo "Done testing"

full-build: clean deps CitrusEngine
	@${MAKE} --no-print-directory test && ([ $$? -eq 0 ] && ${MAKE} --no-print-directory CitrusPlayground)

help:
	@echo "Command usage: make [config=name] [renderapi=api] [command]"
	@echo ""
	@echo "Available configurations:"
	@echo "  debug"
	@echo "  release"
	@echo ""
	@echo "Available rendering APIs:"
	@echo "  OpenGL"
	@echo ""
	@echo "Available commands:"
	@echo "   all (build all, default) (rendering API defaults to OpenGL if not explicitly set)"
	@echo "   clean (clean build files)"
	@echo "   run (run CitrusPlayground debug build)"
	@echo "   run-release (run CitrusPlayground release build)"
	@echo "   build-run (build all, then run CitrusPlayground debug build) (rendering API defaults to OpenGL if not explicitly set)"
	@echo "   test (build and run tests)"
	@echo "   full-build (run a clean build of CitrusEngine and dependencies, test CitrusEngine, and build CitrusPlayground if tests succeed) (rendering API defaults to OpenGL if not explicitly set)"
	@echo "   GLFW (build GLFW)"
	@echo "   ImGui (build ImGui)"
	@echo "   Glad (build Glad)"
	@echo "   CitrusEngine (build Citrus Engine) (rendering API defaults to OpenGL if not explicitly set)"
	@echo "   CitrusPlayground (build Citrus Playground) (rendering API defaults to OpenGL if not explicitly set)"
	@echo ""
	@echo "See the GitHub repo here: https://github.com/RobotLeopard86/CitrusEngine"
	@echo "Thank you for contributing to Citrus Engine."