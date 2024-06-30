# Backend `gl-glfw` Info

## Limitations
* GLFW's Wayland support doesn't allow moving windows programmatically, so going back to windowed mode from fullscreen or borderless will not position windows where they were prior to leaving windowed mode.

## Dependencies
Windows and macOS: Platform-specific SDKs  
GNU/Linux: Install the packages below based on your distro
* Fedora-based (e.g. RHEL) Linux: `wayland-devel wayland-protocols-devel libxkbcommon-devel libXcursor-devel libXi-devel libXinerama-devel libXrandr-devel extra-cmake-modules`
* Debian-based (e.g. Ubuntu) Linux: `libwayland-dev libxkbcommon-dev xorg-dev wayland-protocols`
* Arch-based (e.g. Manjaro) Linux: `pacman -S libglvnd libxkbcommon extra-cmake-modules libxcursor libxi libxinerama libxrandr mesa wayland-protocols`  
