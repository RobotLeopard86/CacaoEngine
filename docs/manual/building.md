# Build Instructions

## Prerequisites
* Git
* Meson
* Ninja
* Python 3
* CMake
* GLSLC
* **(Windows/macOS only)** Platform-specific SDKs
* **Your backend of choice may have additional requirements!** See its page for details on what else you may need to install.

## Linux Package Dependencies
Cacao Engine uses OpenAL-Soft, a software implementation of the OpenAL API which sits atop platform-specific APIs. On Linux, depending on the audio providers you want available, you will need to install additional packages prior to configuring. Find these in the table below.  
| Provider | Debian-based | Fedora-based | Arch-based |
| -------- | ------------ | ------------ | ---------- |
| Pipewire | `libpipewire-0.3-dev` | `pipewire-devel` | `libpipewire` |
| PulseAudio | `libpulse-dev` | `pulseaudio-libs pulseaudio-libs-devel` | `libpulse` |
| ALSA | `libasound2-dev` | `alsa-lib alsa-lib-devel` | `alsa-lib` |  
Cacao Engine also requires additional system packages. By base distro, these are:
* Fedora-based (e.g. RHEL): `wayland-devel`, `wayland-protocols-devel`, `libxkbcommon-devel`, `libXcursor-devel`, `libXfixes-devel`, `libXi-devel`, `libXinerama-devel`, `libXrandr-devel`, `libdecor-devel`, `libdrm-devel`, `ibus-devel`, `fcitx-devel`, `libuuid-devel`
* Debian-based (e.g. Ubuntu): `libwayland-dev`, `libxkbcommon-dev`, `libxfixes-dev`, `xorg-dev`, `wayland-protocols`, `libdecor0-dev`, `libdrm-dev`, `libibus-1.0-dev`, `fcitx`, `uuid-dev`
* Arch-based (e.g. Manjaro): `libglvnd`, `libxkbcommon`, `extra-cmake-modules`, `libxcursor`, `libxi`, `libxfixes`, `libxinerama`, `libxrandr`, `mesa`, `wayland-protocols`, `libibus`, `libdrm`, `fcitx`, `util-linux-libs`

## Compilers
Cacao Engine is designed to build **ONLY** with Clang (including Apple's modified Clang) and LLD. If problems are encountered when using alternative compilers or linkers, they will not be officially supported. While other compilers may work, they are officially not supported.

## Known Building Platforms
* x86_64 GNU/Linux with Clang and LLD
* aarch64 GNU/Linux with Clang and LLD
* x86_64 Windows with MSVC
* aarch64 macOS with Clang and LLD

## 1. Configuring the Build
Select a backend from [the backends list](backends). Enter the source tree root directory in your terminal, then run the following command `meson setup <build directory> --native-file native.ini -Dgraphics_backend=<chosen backend>`. If on Windows, you may add `-Dwindows_noconsole=true` to the command to configure a build without a console. You can also customize the name of the Cacao Engine executable by adding `-Dexe_name=(your name of choice)` to the command. Meson runs debug builds by default. See [this page](https://mesonbuild.com/Builtin-options.html#core-options) on Meson's website for the list of types. This command may take some time to complete, especially if this is a first-time build of the engine.

## 2. Build
Congratulations, you're ready to build. Simply enter the build directory in your terminal and run `ninja`. Wait for it to complete, and then you're basically good to go. The `cacaoengine` executable on its own won't workif the proper files for starting a game aren't found. See the rest of the documentation for how to set things up.
