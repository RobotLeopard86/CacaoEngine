# Backend `gles-glfw`

This backend uses OpenGL ES 3.0 and GLFW.

## Limitations
* 64-bit shader uniform types (`int64`, `uint64`, and `double`) are not allowed
* Boolean shader uniform types break and will not apply properly, use `int` or `uint` instead

## Linux Dependencies
* Fedora-based (e.g. RHEL) Linux: wayland-devel wayland-protocols-devel libxkbcommon-devel libXcursor-devel libXi-devel libXinerama-devel libXrandr-devel extra-cmake-modules
* Debian-based (e.g. Ubuntu) Linux: libwayland-dev libxkbcommon-dev xorg-dev wayland-protocols
* Arch-based (e.g. Manjaro) Linux: libglvnd libxkbcommon extra-cmake-modules libxcursor libxi libxinerama libxrandr mesa wayland-protocols
