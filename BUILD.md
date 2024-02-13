# Cacao Engine Contributors Info

## VERY IMPORTANT
* Cacao Engine will currently only know to build correctly on GNU/Linux using C++20.
* You may need to install certain development packages if you are on GNU/Linux for GLFW and OpenGL. Refer to (the GLFW docs)[https://www.glfw.org/docs/latest/compile_guide.html#compile_deps] for the most up-to-date information (When targeting Linux, Cacao Engine will build for both X11 and Wayland so install both sets of dependencies)
* This project contains Git submodules. Make sure to clone recursively.
* Run the appropriate setup script for your platform (`setup_posix.sh` for Linux/macOS, Windows is currently unsupported) prior to attempting to build.

## Setup
Cacao Engine contains dependencies which are built through CMake using the Ninja generator (to ensure cross-platform compatibility). While the setup script does have the necessary command line parameters, you will need to install CMake and Ninja to build these dependencies.

## Build System
Cacao Engine uses GNU Make and Clang as the build system.

## Prerequisites
* GNU Make
* Clang

## Makefiles
Simply `cd` to the directory of the component you want to build and run `make`.

## Contributing Your Changes
To contribute your changes, simply fork the repository, make your changes, and submit a pull request. It's that simple.