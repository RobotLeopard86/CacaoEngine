# Backend `gl-sdl` Info

## Limitations
* The grave/tilde and F25 keys are not detected as key input

## Dependencies
Windows and macOS: Platform-specific SDKs  
GNU/Linux: Install the packages below based on your distro
* Fedora-based (e.g. RHEL) Linux: `wayland-devel wayland-protocols-devel libxkbcommon-devel libXcursor-devel libXfixes-devel libXi-devel libXinerama-devel libXrandr-devel libdecor-devel libdrm-devel ibus-devel fcitx-devel`
* Debian-based (e.g. Ubuntu) Linux: `libwayland-dev libxkbcommon-dev libxfixes-dev xorg-dev wayland-protocols libdecor0-dev libdrm-dev libibus-1.0-dev fcitx`
* Arch-based (e.g. Manjaro) Linux: `pacman -S libglvnd libxkbcommon extra-cmake-modules libxcursor libxi libxfixes libxinerama libxrandr mesa wayland-protocols libibus libdrm fcitx`