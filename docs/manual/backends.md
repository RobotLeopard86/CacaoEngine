# Backend Info

This page lists information about the available graphics backends for Cacao Engine.

## OpenGL
OpenGL (no GLES support) version 4.1, Core Profile. This version was chosen as it is the last well-supported OpenGL version available on macOS.  
In the future, it may be possible that some advanced features that may be added (e.g. compute shaders) may not be available using this backend, but until then you can expect complete feature parity.

## Vulkan
Vulkan version 1.3 or later. The `VK_KHR_swapchain` (almost universally available) and `VK_KHR_push_descriptor` (no longer needed since Vulkan 1.4) extensions are required.  
In debug builds, you can set the following environment variables when using the Vulkan backend to manage diagnostics:
* `CACAO_DISABLE_VULKAN_VALIDATION`: set to `YES` to disable Vulkan validation layers
* `CACAO_ENABLE_APIDUMP`: set to `YES` to enable API dumping of Vulkan calls (warning: produces a lot of output)

## Compatibility Matrix
| Backend | Windows | Mac | Linux* |
| -- | ------- | --- | ------ |
| OpenGL | ✅ | ✅ | ✅ |
| Vulkan | 🟨 | ❌ | 🟨  |  

\* Most Linux testing occurs on Fedora Linux. Other distributions may have different compatibilities.

#### Legend
* ✅ - Fully working and supported
* 🟨 - Intended to work but untested / not fully working
* ❌ - Incompatible with platform / will not be supported
