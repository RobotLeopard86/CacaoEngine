# Vulkan headers
vkheaders = subproject('vulkan-headers', required: true).get_variable('vulkan_headers_dep')
vkhpp = subproject('vulkan-hpp', required: true).get_variable('vulkan_hpp_dep')

# VulkanMemoryAllocator
vma = subproject('vma', required: true).get_variable('vma_dep')
vmahpp = subproject('vma-hpp', required: true).get_variable('vma_hpp_dep')

# Sources
vk_sources = [
	'VulkanModule.cpp'
]
vk_inc = [engine_headers, module_private_headers]

# Graphics connection
if host_machine.system() == 'windows'
    vk_sources += 'Win32WSI.cpp'
	vk_inc += '..' / 'win32'
elif host_machine.system() == 'linux'
	vk_inc += '..' / 'linux'
    if linux_windowing.has('x11')
		vk_sources += 'XWSI.cpp'
	endif
	if linux_windowing.has('wayland')
		vk_sources += 'WaylandWSI.cpp'
	endif
else
    error('Unsupported platform for Vulkan module!')
endif

# Vulkan module
vkmodule = static_library('cacaovk', sources: vk_sources, include_directories: vk_inc, 
	dependencies: [vkheaders, vkhpp, vma, vmahpp, glm_dep], cpp_args: ['-DVULKAN_HPP_NO_NODISCARD_WARNINGS', 
	'-DVULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1', '-DVMA_VULKAN_VERSION=1003000', '-DVMA_STATIC_VULKAN_FUNCTIONS=0', 
	'-DVMA_DYNAMIC_VULKAN_FUNCTIONS=1'], pic: true, install: true, install_tag: 'cacao')
vk_dep = declare_dependency(link_whole: vkmodule)
gfx_deps += vk_dep