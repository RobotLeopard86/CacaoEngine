#ifdef __linux__
#ifdef HAS_X11
#define VK_USE_PLATFORM_XCB_KHR
#endif
#ifdef HAS_WAYLAND
#define VK_USE_PLATFORM_WAYLAND_KHR
#endif
#endif
#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif


#include "vulkan/vulkan.hpp"

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.hpp"// IWYU pragma: keep

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE