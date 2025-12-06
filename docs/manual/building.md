# Build Instructions

These instructions will guide you through the process of building Cacao Engine.  
```{attention}
Before you begin, please check the [Platform Support page](./platforms) to ensure you have a compatible system!
```

## Prerequisites

#### All Platforms
* Git
* Meson (version `1.9.0` or newer)
* Ninja
* Python 3
* CMake

```{note}
Some Linux distros may not have the requisite Meson version available in their repositories. Follow [the instructions here](https://mesonbuild.com/Getting-meson.html#installing-meson-with-pip) to install via `pip` instead, which always has the latest versions.
```

#### Windows
Install the Windows SDK via [Visual Studio](https://visualstudio.microsoft.com) or [direct download](https://developer.microsoft.com/en-us/windows/downloads/windows-sdk).

#### macOS
Install [Xcode](https://developer.apple.com/xcode) or the Command Line Tools for Developers (run `xcode-select --install` in your terminal to install) to get the macOS SDK.

#### Fedora Linux and derivatives (e.g. RHEL, Nobara):
Run the following command to install all necessary system packages (you may need `sudo` privileges):
`dnf install wayland-devel wayland-protocols-devel libxkbcommon-devel libxcb-devel xcb-util-devel xcb-util-wm-devel xcb-util-keysyms-devel xorg-x11-proto-devel libdrm-devel libuuid-devel pipewire-devel pulseaudio-libs-devel alsa-lib alsa-lib-devel libglvnd-devel`

#### Debian GNU/Linux and derivatives (e.g. Ubuntu, Mint, Pop!_OS):
Run the following command to install all necessary system packages (you may need `sudo` privileges):
`apt install libwayland-dev wayland-protocols libxkbcommon-dev xorg-dev libxcb1-dev libxcb-icccm4-dev libxcb-keysyms1-dev libxcb-randr0-dev libxcb-util-dev libxcb-xkb-dev x11proto-dev libdrm-dev uuid-dev libpipewire-0.3-dev libpulse-dev libasound2-dev libglvnd-dev`

#### Arch Linux and derivatives (e.g. Manjaro, Garuda, EndeavourOS):
Run the following command to install all necessary system packages (you may need `sudo` privileges):
`pacman -S libglvnd mesa libxkbcommon wayland-protocols libxcb xcb-util xcb-util-keysyms xcb-util-wm xorgproto util-linux-libs libdrm libpipewire libpulse alsa-lib`

#### Linux Notes
You may be able to use a subset of the listed packages by disabling some features. For example, the Wayland-related packages are not needed if only building for X11, and the XCB packages are not needed if only building for Wayland.

Similarly, you may not need Pipewire packages or ALSA packages if not targeting those audio subsystems, though it is recommended to always include them for best compatibility.

## Compiler Support
Cacao Engine is primarily built and tested using [Clang](https://clang.llvm.org) with the [LLD linker](https://lld.llvm.org). While it should compile using GCC or MSVC or alternate linkers, no compatibility guarantees are assured. Cacao Engine is currently confirmed to build correctly on Windows, macOS, and Fedora Linux using Clang and LLD. More compatibility tests will be conducted prior to release.  

## Build
In the root Cacao Engine directory, run the following command to configure the build:  
macOS/Linux: `meson setup <build directory> --native-file native.ini [--buildtype release] -Dbackends=<graphics backends>`.  
Windows    : `meson setup <build directory> --vsenv --native-file native.ini [--buildtype release] -Dbackends=<graphics backends>`.

The available backends are `opengl` and `vulkan`. See the [backends info page](./backends) for more details. You can also choose to build the sandbox application by adding `-Dbuild_sandbox=true` to the command line.  

On Linux, the supported windowing systems can be selected via the `linux_windowing` option. By default, this is set to `x11,wayland` to include both, but you can choose to exclude one if you wish.

Windows builds also have the option of disabling the automatic console window by setting `-Dwindows_noconsole=true` on the command line.

For a complete list of options, please see the file `meson_options.txt` in the root directory.  

Please note that configuration may take a long time, especially in a fresh checkout of the repository, since all non-system dependencies will be downloaded at this time (some of which are large).  

Once configuration is complete, run `meson compile -C <build directory>` to build the engine. Like configuration, this will take a while, as the dependencies contribute a few thousand objects (mostly from LLVM in the reflection generator). If you chose to build the sandbox application, it can be found in the `sandbox` directory in your build directory.

When using Wayland, window decorations may fail to load due to the same issue. For this reason, it is advised to first run `meson devenv` in your build directory to set up the proper paths.

## Visual Studio and Xcode Projects
Cacao Engine has not yet been tested with Meson's Visual Studio or Xcode backends. It is not guaranteed to work, but feel free to try it out for yourself. Official guidance will be provided closer to release when testing begins.
