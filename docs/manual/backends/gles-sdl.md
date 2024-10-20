# Backend `gles-sdl`

This backend uses OpenGL ES 3.0 and SDL.

## Limitations
* 64-bit shader uniform types (`int64`, `uint64`, and `double`) are not allowed
* Boolean shader uniform types break and will not apply properly, use `int` or `uint` instead
* For some reason, the SDL header does not contain the grave/tilde key (`` ` ``/`~`), so it won't be detected as key input
* SDL does not support the F25 key (if you have that for some reason), so it won't be detected as key input

## Dependencies
* Fedora-based (e.g. RHEL) Linux: wayland-devel wayland-protocols-devel libxkbcommon-devel libXcursor-devel libXfixes-devel libXi-devel libXinerama-devel libXrandr-devel libdecor-devel libdrm-devel ibus-devel fcitx-devel
* Debian-based (e.g. Ubuntu) Linux: libwayland-dev libxkbcommon-dev libxfixes-dev xorg-dev wayland-protocols libdecor0-dev libdrm-dev libibus-1.0-dev fcitx
* Arch-based (e.g. Manjaro) Linux: libglvnd libxkbcommon extra-cmake-modules libxcursor libxi libxfixes libxinerama libxrandr mesa wayland-protocols libibus libdrm fcitx
