# Backend `gles-al-glfw` Info

## Dependencies
Windows and macOS: Platform-specific SDKs  
GNU/Linux: Install the packages below based on your distro
* Fedora-based (e.g. RHEL) Linux: `wayland-devel wayland-protocols-devel libxkbcommon-devel libXcursor-devel libXi-devel libXinerama-devel libXrandr-devel extra-cmake-modules`
* Debian-based (e.g. Ubuntu) Linux: `libwayland-dev libxkbcommon-dev xorg-dev wayland-protocols`
* Arch-based (e.g. Manjaro) Linux: `pacman -S libglvnd libxkbcommon extra-cmake-modules libxcursor libxi libxinerama libxrandr mesa wayland-protocols`  

Additionally, depending on the audio providers you want available, you will need to install additional packages. Find these in the table below.  
| Provider | Debian-based | Fedora-based | Arch-based |
| -------- | ------------ | ------------ | ---------- |
| Pipewire | `libpipewire-0.3-dev` | `pipewire-devel` | `libpipewire` |
| PulseAudio | `libpulse-dev` | `pulseaudio-libs pulseaudio-libs-devel` | `libpulse` |
| ALSA | `libasound2-dev` | `alsa-lib alsa-lib-devel` | `alsa-lib` |