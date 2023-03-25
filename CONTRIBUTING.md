# Citrus Engine Contributors Info

## VERY IMPORTANT
* Citrus Engine will currently ONLY build on GNU/Linux using C++20.
* You may need to install certain development packages if you are on GNU/Linux (e.g. X11 and Wayland packages), or the project may not build.

## Environment
Citrus Engine uses Visual Studio Code as a workspace environment (though you are not required to use it, it is advised to use it to maintain consistency), with GNU Make and Zig being the backbone of the build system.

## Prerequisites
* GNU Make
* Zig

## Tools
GNU Make should be pre-installed on your GNU/Linux distro. If it isn't, you should be able to install it from your package manager. Windows users can use [Chocolatey](https://chocolatey.org/) to install GNU Make with this command: `choco install make`, and macOS users can install it with brew, with this command: `brew install make`. You will also need Zig (the compiler of choice for Citrus Engine). Technically, Zig is only used for cross-compilation, but it should be used for all compilation regardless. Download Zig [here](https://ziglang.org/download/). Do not use the package manager step, most package managers will not have an up-to-date version of Zig with the custom linker used for cross-compilation. Lastly, since Visual Studio Code is the preferred environment, you should download that [here](https://code.visualstudio.com).

## Contributing Your Changes
To contribute your changes, simply fork the repository, make your changes, and submit a pull request. It's that simple.