# Citrus Engine Contributors Info

## VERY IMPORTANT
* Citrus Engine will currently ONLY build on GNU/Linux using C++20.
* You may need to install certain development packages if you are on GNU/Linux (do note that these are X11 packages, Citrus Engine currently does not support Wayland), as the project will not build without them.
    * On Debian and its derivatives (e.g. Ubuntu, Linux Mint), the  `xorg-dev` package is required
    * On Fedora and its derivatives (e.g. CentOS, Red Hat), the `libXcursor-devel`, `libXi-devel`, `libXinerama-devel`, and `libXrandr-devel` packages are required

## Build System
Citrus Engine uses GNU Make and Zig as the build system.

## Prerequisites
* GNU Make
* Zig

## Tools
GNU Make should be pre-installed on your GNU/Linux distro. If it isn't, you should be able to install it from your package manager. Windows users can use [Chocolatey](https://chocolatey.org/) to install GNU Make with this command: `choco install make`, and macOS users can install it with brew, with this command: `brew install make`. You will also need Zig's C/C++ compiler, which is the default (but can be overriden with the CC and CXX flags in the top-level Makefile). Download Zig [here](https://ziglang.org/download/). You can use the package manager step. You will need to add Zig to your PATH environment variable so that GNU Make can locate it.

## Environment Set Up
Run the `setupEnvironment.sh` script in the root directory to set everything up (make sure to set it as executable!). **THIS IS REQUIRED BEFORE BUILDING!!!**
The recommended IDE is Visual Studio Code, which can be downloaded [here](https://code.visualstudio.com). The project comes with a preset `launch.json` file for debugging as well.

## Makefile Commands
You can use `make help` to see the command list, but here is a quick reference:
* `make` or `make all`: Builds the engine, playground, and required libraries
* `make clean`: Cleans build files
* `make CitrusEngine`: Builds only the engine and required libraries
* `make run`: Runs the latest debug build of the playground
* `make run-release`: Runs the latest release build of the playground
* `make build-run`: Builds the engine, playground, and required libraries, then runs the latest debug build of the playground
* `make test`: Builds the engine and required libraries, then runs tests found in the [CitrusEngine/tests](CitrusEngine/tests) directory
* `make full-build`: Cleans build files, rebuilds the engine and required libraries, then runs tests. If tests succeed, then CitrusPlayground is built
* `make clean-build`: Cleans and rebuilds everything

## Contributing Your Changes
To contribute your changes, simply fork the repository, make your changes, and submit a pull request. It's that simple.