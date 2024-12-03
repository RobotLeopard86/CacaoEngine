# Backend `vk-glfw`

This backend uses Vulkan 1.2 with some extensions and GLFW.

## Known Issues
* Minimizing the window on Wayland causes a crash. Unknown if this occurs on X11.

## Vulkan Extension Requirements
* `VK_KHR_swapchain`
* `VK_KHR_dynamic_rendering`
* `VK_KHR_synchronization2`
* `VK_KHR_dedicated_allocation`
* `VK_KHR_copy_commands2`
* `VK_KHR_get_memory_requirements2`
* `VK_KHR_push_descriptor`
* `VK_EXT_extended_dynamic_state`
* `VK_EXT_extended_dynamic_state3`
* `VK_EXT_robustness2`

## Dependencies
* All Platforms: Vulkan SDK
* Fedora-based (e.g. RHEL) Linux: Packages `wayland-devel`, `wayland-protocols-devel`, `libxkbcommon-devel`, `libXcursor-devel`, `libXi-devel`, `libXinerama-devel`, `libXrandr-devel`, `extra-cmake-modules`
* Debian-based (e.g. Ubuntu) Linux: Packages `libwayland-dev`, `libxkbcommon-dev`, `xorg-dev`, `wayland-protocols`
* Arch-based (e.g. Manjaro) Linux: Packages `libglvnd`, `libxkbcommon`, `extra-cmake-modules`, `libxcursor`, `libxi`, `libxinerama`, `libxrandr`, `mesa`, `wayland-protocols`