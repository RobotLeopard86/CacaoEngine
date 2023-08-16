# Citrus Engine Contributors Info

## VERY IMPORTANT
* Citrus Engine will currently ONLY build on GNU/Linux using C++20.
* Citrus Engine currently does not support Wayland, only X11.
* You may need to install certain development packages if you are on GNU/Linux, as the project will not build without them.
    * On Debian and its derivatives (e.g. Ubuntu, Linux Mint), the  `xorg-dev` package is required
    * On Fedora and its derivatives (e.g. CentOS, Red Hat), the `libXcursor-devel`, `libXi-devel`, `libXinerama-devel`, and `libXrandr-devel` packages are required
    * On Arch and its derivatives (e.g. Manjaro), the `libglvnd`, `libxi`, `libxinerama`, `libxrandr`, `libxcursor`, `libxkbcommon`, and `mesa` packages are required

## Build System
Citrus Engine uses GNU Make and Zig as the build system.

## Environment Set Up (**REQUIRED**)
Run the `setupEnvironment.sh` script in the root directory to set everything up (make sure to set it as executable!).

## Prerequisites
* GNU Make
* Zig

## Installing Make
GNU Make should be pre-installed on your GNU/Linux distro. If it isn't, you should be able to install it from your package manager. Windows users can use [Chocolatey](https://chocolatey.org/) to install GNU Make with this command: `choco install make`, and macOS users can install it with brew, with this command: `brew install make`.

## Installing Zig
While not required, the default compiler is Zig's C/C++ compiler (but this can be overriden with the `c` and `cpp` flags in the top-level Makefile). Download Zig [here](https://ziglang.org/download/) or install from your package manager. Make sure the `zig` executable is in your PATH environment variable so that Make can locate it.

## Makefile
It is **HIGHLY RECOMMENDED** to use `make help` prior to using the Makefile in this project, as it does not function identically to other Make-based projects.

## Contributing Your Changes
To contribute your changes, simply fork the repository, make your changes, and submit a pull request. It's that simple.