# Cacao Engine Contributors Info

## VERY IMPORTANT
* Cacao Engine is currently only known to build correctly on GNU/Linux using C++20.
* You may need to install certain development packages if you are on GNU/Linux for GLFW and OpenGL. Refer to (the GLFW docs)[https://www.glfw.org/docs/latest/compile_guide.html#compile_deps] for the most up-to-date information (When targeting Linux, Cacao Engine will build for both X11 and Wayland so install both sets of dependencies)
* This project contains Git submodules. Make sure to clone recursively.

## Prerequisites
* Ninja
* CMake (for dependencies)
* Clang

## Setup
Cacao Engine contains dependencies which are built through CMake using the Ninja generator. Run the appropriate setup script for your platform (`setup_posix.sh` for Linux/macOS, Windows is currently unsupported) prior to attempting to build.

## Build System
Cacao Engine uses Ninja and Clang as the build system.

## Building
Simply `cd` to the directory of the component you wish to build. **IMPORTANT:** For the engine core, ensure to create a symlink (or shortcut, not sure yet if that works) to the `build` directory of your desired backend in the root project directory named exactly `selected-backend-bin`.

## Contributing Your Changes
To contribute your changes, simply fork the repository, make your changes, and submit a pull request. It's that simple.