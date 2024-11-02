# Cacao Engine Environment Variables

Cacao Engine will obey the following environment variables if they are set to `YES` (case-sensitive):
* (**Linux Only**) `CACAO_FORCE_X11` - Forces the engine to run under X11
  
Cacao Engine also has the following environment variables that only exist in debug builds:
* (**Vulkan Backend Only**) `CACAO_DISABLE_VULKAN_VALIDATION` - Disables the Vulkan validation layers (can improve performance)
* (**Vulkan Backend Only**) `CACAO_ENABLE_APIDUMP` - Dumps Vulkan API calls to the console. Probably not needed unless debugging.