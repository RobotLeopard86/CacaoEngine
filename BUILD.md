# Cacao Engine Build Instructions

## VERY IMPORTANT
* Cacao Engine is currently only known to build correctly on x86_64 GNU/Linux. While it may build on other platforms, it has not been tested.
* Cacao Engine is currently only known to build correctly using Clang and LLD. While it may build with other compilers/linkers, it has not been tested.
* This project contains Git submodules. Make sure to clone recursively.

## Prerequisites
* Meson
* Ninja
* CMake (for dependencies)
* (reccommended but optional) Clang
* (reccommended but optional) LLD
* GLSLC (for playground)

## 1. Environment Set-Up
To build Cacao Engine, the prerequisites listed above are required. You can install Meson as listed on their website [here](https://mesonbuild.com/SimpleStart.html#installing-meson). CMake is also simple. On Linux, it should be available in your package manager. For Windows and macOS users, you can get CMake on their website's [download page](https://cmake.org/download/#latest). Clang and LLD, though optional, are reccommended and are the compilers that Cacao Engine is designed to work on. Like CMake, they should be available in your Linux package manager, and can be downloaded on Windows and macOS from [the LLVM website](https://releases.llvm.org/). Finally, GLSLC is available as part of the Vulkan SDK (download that [here](https://vulkan.lunarg.com/sdk/home)), or can be downloaded standalone from the [project GitHub](https://github.com/google/shaderc/blob/main/downloads.md).

## 2. Configuring the Build
This section presumes that you already have a copy of the source code. If you do not, either download a [.zip from GitHub](https://github.com/RobotLeopard86/CacaoEngine/archive/refs/heads/engine2.zip) or install Git and clone the repository. At this time, please select a backend from [the backends list](BACKENDS.md). Enter the source tree root directory in your terminal, then run the following command `meson setup <build directory> --native-file default_native.ini [--debug] -Dbuild_playground=true|false -Duse_backend=<chosen backend>`. Only add the `--debug` flag if you want a debug build, and set the `build_playground` option to `true` if you want to optionally build the playground, or `false` otherwise. If on Windows, you may add `-Dwindows_noconsole=true` to the command to configure a build without a console. This command may take some time to complete.

## 3. Build
Congratulations, you're ready to build. Simply enter the build directory in your terminal and run `ninja`. Wait for it to complete, and then you're basically good to go. The `cacaoengine` executable on its own won't display anything if the proper files for starting a game aren't found, so see the the following section in which the playground is set up as an example.

## 4. Playground Setup
If you chose to build the playground, you will find an additional `playground` directory in your build folder. This directory contains the built files produced by the playground. This won't run on its own though, so to set that up, run `ninja bundle_playground` in your terminal. This will merge the playground, its assets, and the engine binaries into one directory for your convenience. If you would like to do something similar with your own game build, make a new directory, and copy your `launch.so|.dll|.dylib` (whichever you have) to it, as well as the `cacaoengine[.exe]` in the `cacao` directory and the `libcacaobackend.so|.dll|.dylib` (whichever you have) in the `backends/<chosen backend>` directory. Additionally copy your other project directories, and you're done.