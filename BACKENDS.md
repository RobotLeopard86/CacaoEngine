# Cacao Engine Backend Reference

| ID | What | Windows | Mac | Linux |
| -- | ---- | ------- | --- | ----- |
| gl-al-glfw | OpenGL 3.3 Core Profile, GLFW, OpenAL | ✅ | ✅ | ✅ |

### Backend Dependency Info
gl-al-glfw:
* Fedora-based (e.g. RHEL) Linux: `dnf install wayland-devel wayland-protocols-devel libxkbcommon-devel libXcursor-devel libXi-devel libXinerama-devel libXrandr-devel extra-cmake-modules`
* Debian-based (e.g. Ubuntu) Linux: `apt install libwayland-dev libxkbcommon-dev xorg-dev`
* Arch-based (e.g. Manjaro) Linux: `pacman -S libglvnd libxkbcommon extra-cmake-modules libxcursor libxi libxinerama libxrandr mesa wayland-protocols`
* Windows: Windows SDK
* macOS: macOS SDK