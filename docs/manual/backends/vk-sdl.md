# Backend `vk-sdl`

This backend uses Vulkan 1.3 with some extensions and SDL (Simple DirectMedia Layer).

## Limitations
* The F25 key is not detected as key input

## Vulkan Extension Requirements
* `VK_KHR_swapchain`
* `VK_KHR_push_descriptor`
* `VK_EXT_extended_dynamic_state3`
* `VK_EXT_robustness2`

## Dependencies
* All Platforms: Vulkan SDK
* Fedora-based (e.g. RHEL) Linux: Packages `wayland-devel`, `wayland-protocols-devel`, `libxkbcommon-devel`, `libXcursor-devel`, `libXfixes-devel`, `libXi-devel`, `libXinerama-devel`, `libXrandr-devel`, `libdecor-devel`, `libdrm-devel`, `ibus-devel`, `fcitx-devel`
* Debian-based (e.g. Ubuntu) Linux: Packages `libwayland-dev`, `libxkbcommon-dev`, `libxfixes-dev`, `xorg-dev`, `wayland-protocols`, `libdecor0-dev`, `libdrm-dev`, `libibus-1.0-dev`, `fcitx`
* Arch-based (e.g. Manjaro) Linux: Packages `libglvnd`, `libxkbcommon`, `extra-cmake-modules`, `libxcursor`, `libxi`, `libxfixes`, `libxinerama`, `libxrandr`, `mesa`, `wayland-protocols`, `libibus`, `libdrm`, `fcitx`