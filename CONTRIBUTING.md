# Citrus Engine Contributors Info

## VERY IMPORTANT
* Citrus Engine will currently ONLY build on GNU/Linux using C++20.
* You may need to install certain development packages if you are on GNU/Linux (e.g. X11 and Wayland packages), or the project may not build.

## Build System
Citrus Engine uses GNU Make and Zig as the build system.

## Prerequisites
* GNU Make
* Zig

## Tools
GNU Make should be pre-installed on your GNU/Linux distro. If it isn't, you should be able to install it from your package manager. Windows users can use [Chocolatey](https://chocolatey.org/) to install GNU Make with this command: `choco install make`, and macOS users can install it with brew, with this command: `brew install make`. You will also need Zig (the compiler of choice for Citrus Engine). Technically, Zig is only used for cross-compilation, but it should be used for all compilation regardless and is the default and cannot be overriden. Download Zig [here](https://ziglang.org/download/). Do not use the package manager step, as most package managers will not have an up-to-date version of Zig with the custom linker used for cross-compilation. You will need to add Zig to your PATH environment variable so that GNU Make can locate it.

## Environment Set Up
Run the `setupEnvironment.sh` script in the root directory to set everything up (make sure to set it as executable!). **THIS IS REQUIRED BEFORE BUILDING!!!**

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

## Contributing Your Changes
To contribute your changes, simply fork the repository, make your changes, and submit a pull request. It's that simple.