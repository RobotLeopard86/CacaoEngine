# Backend `vk-glfw`

This backend uses Vulkan 1.2 with some extensions and GLFW.

## Limitations
* Running under Wayland causes a crash immediately after startup. This appears to be a GLFW bug. Run under X11 or XWayland instead.

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
* Vulkan SDK
