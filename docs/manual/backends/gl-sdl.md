# Backend `gl-sdl`

This backend uses OpenGL 4.1 Core Profile and SDL (Simple DirectMedia Layer).

## Limitations
* For some reason, the SDL header does not contain the grave/tilde key (`` ` ``/`~`), so it won't be detected as key input
* SDL does not support the F25 key (if you have that for some reason), so it won't be detected as key input

## Dependencies
* Fedora-based (e.g. RHEL) Linux: wayland-devel wayland-protocols-devel libxkbcommon-devel libXcursor-devel libXfixes-devel libXi-devel libXinerama-devel libXrandr-devel libdecor-devel libdrm-devel ibus-devel fcitx-devel
* Debian-based (e.g. Ubuntu) Linux: libwayland-dev libxkbcommon-dev libxfixes-dev xorg-dev wayland-protocols libdecor0-dev libdrm-dev libibus-1.0-dev fcitx
* Arch-based (e.g. Manjaro) Linux: libglvnd libxkbcommon extra-cmake-modules libxcursor libxi libxfixes libxinerama libxrandr mesa wayland-protocols libibus libdrm fcitx