# Vulkan headers
vkheaders = subproject('vulkan-headers', required: true).get_variable('vulkan_headers_dep')
vkhpp = subproject('vulkan-hpp', required: true).get_variable('vulkan_hpp_dep')

# VulkanMemoryAllocator
vma = subproject('vma', required: true).get_variable('vma_dep')
vmahpp = subproject('vma-hpp', required: true).get_variable('vma_hpp_dep')

# Sources
vk_sources = [
	'VulkanModule.cpp',
	'Utils.cpp',
	'VulkanImpl.cpp'
]
vk_inc = [engine_headers, module_private_headers]
vk_dep = [vkheaders, vkhpp, vma, vmahpp, glm_dep]

# Graphics connection
if host_machine.system() == 'windows'
    vk_sources += 'wsi' / 'Win32WSI.cpp'
	vk_inc += '..' / 'win32'
elif host_machine.system() == 'linux'
	vk_inc += '..' / 'linux'
    if linux_windowing.contains('x11')
		vk_sources +=  'wsi' / 'XWSI.cpp'
		vk_dep += [xcb_dep, xcb_ks_dep, xcb_atom_dep, xcb_randr_dep]
	endif
	if linux_windowing.contains('wayland')
		vk_sources += 'wsi' / 'WaylandWSI.cpp'
		vk_dep += [wayland_client, xkbcommon_dep, libdecor_dep]
	endif
	vk_sources += 'wsi' / 'LinuxWSI.cpp'
else
    error('Unsupported platform for Vulkan module!')
endif

# Vulkan module
vkmodule = static_library('cacaovk', sources: vk_sources, include_directories: vk_inc, 
	dependencies: vk_dep, cpp_args: ['-DVULKAN_HPP_NO_NODISCARD_WARNINGS', 
	'-DVULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1', '-DVMA_VULKAN_VERSION=1003000', '-DVMA_STATIC_VULKAN_FUNCTIONS=0', 
	'-DVMA_DYNAMIC_VULKAN_FUNCTIONS=1', '-Wno-nullability-completeness'], pic: true, install: true, install_tag: 'cacao')
vk_dep = declare_dependency(link_whole: vkmodule)
gfx_deps += vk_dep