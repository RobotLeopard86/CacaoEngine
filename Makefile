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

all: $(PROJECTS)

GLFW:
ifneq (,$(GLFW_config))
	@echo "Building GLFW - $(GLFW_config)..."
	@${MAKE} CXX="zig c++" CC="zig cc" --no-print-directory -C libs/glfw -f Makefile config=$(GLFW_config)
endif

ImGui:
ifneq (,$(ImGui_config))
	@echo "Building ImGui - $(ImGui_config)..."
	@${MAKE} CXX="zig c++" CC="zig cc" --no-print-directory -C libs/imgui -f Makefile config=$(ImGui_config)
endif

Glad:
ifneq (,$(Glad_config))
	@echo "Building Glad - $(Glad_config)..."
	@${MAKE} CXX="zig c++" CC="zig cc" --no-print-directory -C libs/glad -f Makefile config=$(Glad_config)
endif

CitrusEngine: GLFW Glad ImGui
ifneq (,$(CitrusEngine_config))
	@echo "Building CitrusEngine - $(CitrusEngine_config)..."
	@${MAKE} CXX="zig c++" CC="zig cc" --no-print-directory -C CitrusEngine -f Makefile config=$(CitrusEngine_config)
endif

CitrusPlayground: CitrusEngine
ifneq (,$(CitrusPlayground_config))
	@echo "Building CitrusPlayground - $(CitrusPlayground_config)..."
	@${MAKE} CXX="zig c++" CC="zig cc" --no-print-directory -C CitrusPlayground -f Makefile config=$(CitrusPlayground_config)
endif

clean:
	@echo "Cleaning all..."
	@${MAKE} CXX="zig c++" CC="zig cc" --no-print-directory -C libs/glfw -f Makefile clean
	@${MAKE} CXX="zig c++" CC="zig cc" --no-print-directory -C libs/imgui -f Makefile clean
	@${MAKE} CXX="zig c++" CC="zig cc" --no-print-directory -C libs/glad -f Makefile clean
	@${MAKE} CXX="zig c++" CC="zig cc" --no-print-directory -C CitrusEngine -f Makefile clean
	@${MAKE} CXX="zig c++" CC="zig cc" --no-print-directory -C CitrusPlayground -f Makefile clean


run:
	@echo "Running latest Debug build..."
	@Build/CitrusPlayground/Debug/x86_64-linux/Binaries/CitrusPlayground

run-release:
	@echo "Running latest Release build..."
	@Build/CitrusPlayground/Release/x86_64-linux/Binaries/CitrusPlayground

build-run:
	@echo "Building and running..."
	@${MAKE} all
	@${MAKE} run

help:
	@echo "Command usage: make [config=name] [command]"
	@echo ""
	@echo "Available configurations:"
	@echo "  debug"
	@echo "  release"
	@echo ""
	@echo "Available commands:"
	@echo "   all (build all, default)"
	@echo "   clean (clean build files)"
	@echo "   run (run CitrusPlayground debug build)"
	@echo "   run-release (run CitrusPlayground release build)"
	@echo "   build-run (build all, then run CitrusPlayground debug build)"
	@echo "   GLFW (build GLFW)"
	@echo "   ImGui (build ImGui)"
	@echo "   Glad (build Glad)"
	@echo "   CitrusEngine (build Citrus Engine)"
	@echo "   CitrusPlayground (build Citrus Playground)"
	@echo ""
	@echo "See the GitHub repo here: https://github.com/RobotLeopard86/CitrusEngine"
	@echo "Thank you for contributing to Citrus Engine."