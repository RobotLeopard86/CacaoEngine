# Citrus Engine Contributors Info

## VERY IMPORTANT
* Citrus Engine will currently only know to build correctly on GNU/Linux using C++20.
* You may need to install certain development packages if you are on GNU/Linux for GLFW and OpenGL. Refer to (the GLFW docs)[https://www.glfw.org/docs/latest/compile_guide.html#compile_deps] for the most up-to-date information (When targeting Linux, Citrus Engine will build for both X11 and Wayland so install both sets of dependencies)
* This project contains Git submodules. Make sure to clone recursively.
* Run the appropriate setup script for your platform (`setup_posix.sh` for Linux/macOS, `setup_windows.ps1` for Windows) prior to attempting to build.

## Setup
Citrus Engine contains dependencies which are built through CMake using the Ninja generator (to ensure cross-platform compatibility). While the setup script does have the necessary command line parameters, you will need to install CMake and Ninja to build these dependencies.

## Build System
Citrus Engine uses GNU Make and Zig as the build system.

## Prerequisites
* GNU Make
* Zig

## Installing Make
GNU Make should be pre-installed on your GNU/Linux distro. If it isn't, you should be able to install it from your package manager. Windows users can use [Chocolatey](https://chocolatey.org/) to install it, and you can use Homebrew to install it if on macOS.

## Installing Zig
While not required, the default compiler is Zig's C/C++ compiler (but this can be overriden with the `c` and `cpp` flags in the top-level Makefile). Download Zig [here](https://ziglang.org/download/) or install from your package manager **(Warning: There is currently not a package available on Debian and derivative distros, so you will likely need to use their standalone package)**. Make sure the `zig` executable is in your PATH environment variable so that Make can locate it.

## Building


## Makefiles
Simply `cd` to the directory of the component you want to build and run `make`.

## Contributing Your Changes
To contribute your changes, simply fork the repository, make your changes, and submit a pull request. It's that simple.