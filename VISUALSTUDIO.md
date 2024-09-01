# Cacao Engine Visual Studio Instructions

## Explanation
Meson has a Visual Studio project generator. However, the way it handles run targets in that generator is incompatible with how Cacao Engine functions. Specifically, it generates them as build targets, which means the debugger cannot be used properly without some hacks and it is overall a very messy system. That's where this guide comes in, to help you sort everything out!

## Requirements
* Visual Studio 2022
* "Desktop development with C++" and "Game development with C++" workloads installed

## Instructions
1. Run your Meson setup command as said in the [build instructions page](BUILD.md). However, make sure that you do not set the build type, and also configure to the `Debug` directory.
2. Run that command again, except add `--buildtype=debugoptimized` or `--buildtype=release` to the command line and configure to the `Release` directory
3. Run the Python script `vsgen.py` in the `scripts` folder (run this from the root of the Cacao Engine source directory) to generate Visual Studio solution and project files. This will generate files for Visual Studio 2022, so if using an older version you may have trouble importing it.
