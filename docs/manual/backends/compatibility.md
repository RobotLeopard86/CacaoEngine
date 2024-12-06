# Compatibility Matrix

| ID | Windows | Mac | Linux* | More Info |
| -- | ------- | --- | ------ | --------- |
| gl-glfw |  ✅ | ✅ | ✅ | [View](./gl-glfw) |
| gl-sdl | ✅ | ✅ | ✅ | [View](./gl-sdl) |
| vk-glfw | ✅&#8321; | ❌&#8322; | ✅  | [View](./vk-glfw) |
| vk-sdl | ✅&#8321; | ❌&#8322; | ✅  | [View](./vk-sdl) |  
\* Most testing occurs on Fedora Linux. Other distributions may have different compatibilities.

## Legend
* ✅ - Fully working and supported
* 🔷 - Intended to work but currently not working
* 🟨 - Intended to work but untested
* ❌ - Incompatible with platform / will not be supported

## Notes
&#8321; Some Intel integrated GPUs may not present correctly. If this occurs, use the OpenGL backend.  
&#8322; This may work through MoltenVK, but as MoltenVK currently only supports a subset of Vulkan 1.2 features, this is not guaranteed.