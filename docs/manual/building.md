# Build Instructions

## Prerequisites
* Git
* Meson
* Ninja
* Python 3
* CMake
* GLSLC

## Linux Package Dependencies
Cacao Engine uses OpenAL-Soft, a software implementation of the OpenAL API which sits atop platform-specific APIs. On Linux, depending on the audio providers you want available, you will need to install additional packages prior to configuring. Find these in the table below.  
| Provider | Debian-based | Fedora-based | Arch-based |
| -------- | ------------ | ------------ | ---------- |
| Pipewire | `libpipewire-0.3-dev` | `pipewire-devel` | `libpipewire` |
| PulseAudio | `libpulse-dev` | `pulseaudio-libs pulseaudio-libs-devel` | `libpulse` |
| ALSA | `libasound2-dev` | `alsa-lib alsa-lib-devel` | `alsa-lib` |  

Cacao Engine also requires libuuid. The packages for that are listed below:
* Debian-based: `uuid-dev`
* Fedora-based: `libuuid-devel`
* Arch-based: `util-linux-libs`


## Compilers
Cacao Engine is designed to build using Clang with LLD on Linux and macOS. On Windows, MSVC is the target. If problems are encountered when using alternative compilers (e.g. GCC or Clang with Gold linker), they are officially unsupported. Clang (including Apple's modified Clang) with LLD and MSVC are the ONLY supported compilers. Regarding installation of these compilers, MSVC is available through Visual Studio, which can be downloaded [here](https://visualstudio.microsoft.com). In the Visual Studio Installer, select the `Desktop development with C++` workload before installing, or in the `Modify` UI after. As for Linux, they should be available in your package manager. On macOS, you'll need the Xcode command line tools (install with `xcode-select --install`), and then install the `llvm` and `lld` [Homebrew](https://brew.sh) packages. That will install Clang and LLD to your system. While other compilers may work, they are officially not supported.

## Known Building Platforms
* x86_64 GNU/Linux with Clang and LLD
* aarch64 GNU/Linux with Clang and LLD
* x86_64 Windows with MSVC

## 1. Prerequisite Installation
To build Cacao Engine, you need the prerequisites listed above.  
To install Git, either find it in your package manager if on Linux, with [Homebrew](https://brew.sh) on macOS, or from the [Git for Windows project](https://gitforwindows.org) on Windows.  
You can install Meson as listed on their website [here](https://mesonbuild.com/SimpleStart.html#installing-meson). Instructions for installing Python 3 and ninja can also be found there.  
CMake is also simple. On Linux, it should be available in your package manager. For Windows and macOS users, you can get CMake on their website's [download page](https://cmake.org/download/#latest).  
Finally, GLSLC is available as part of the Vulkan SDK (download that [here](https://vulkan.lunarg.com/sdk/home)), or can be downloaded standalone from the [project GitHub](https://github.com/google/shaderc/blob/main/downloads.md).

## 2. Configuring the Build
**IMPORTANT**: MSVC likely will only be detected properly if run in the Visual Studio Developer Command Prompt/PowerShell.
Select a backend from [the backends list](backends). Enter the source tree root directory in your terminal, then run the following command `meson setup <build directory> -Dbuild_playground=true|false -Duse_backend=<chosen backend>`. **IMPORTANT**: Add `--native-file posix_native.ini` to your command line if on Linux or macOS. Set the `build_playground` option to `true` if you want to optionally build the playground, or `false` otherwise. If on Windows, you may add `-Dwindows_noconsole=true` to the command to configure a build without a console. Meson runs debug builds by default. See [this page](https://mesonbuild.com/Builtin-options.html#core-options) on Meson's website for the list of types. This command may take some time to complete.

## 3. Build
Congratulations, you're ready to build. Simply enter the build directory in your terminal and run `ninja`. Wait for it to complete, and then you're basically good to go. The `cacaoengine` executable on its own won't display anything if the proper files for starting a game aren't found, so see the the following section in which the playground is set up as an example.

## 4. Playground Setup
If you chose to build the playground, you will find an additional `playground` directory in your build folder. This directory contains the built files produced by the playground. This won't run on its own though, so to set that up, run `ninja bundle` in your terminal. This will merge the playground, its assets, and the engine binaries into one directory so that the engine will run properly.