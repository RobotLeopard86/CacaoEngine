# Vulkan headers
vkheaders = subproject('vulkan-headers', required: true).get_variable('vulkan_headers_dep')
vkhpp = subproject('vulkan-hpp', required: true).get_variable('vulkan_hpp_dep')

# VulkanMemoryAllocator
vma = subproject('vma', required: true).get_variable('vma_dep')
vmahpp = subproject('vma-hpp', required: true).get_variable('vma_hpp_dep')

# Add base OpenGL module info
engine_srcs += files(
	'GPU.cpp',
	'Swapchain.cpp',
	'VulkanModule.cpp',
	'VulkanImpl.cpp',
	'Commands.cpp',
	'impls' / 'VulkanMesh.cpp',
	'impls' / 'VulkanTex2D.cpp',
	'impls' / 'VulkanCubemap.cpp',
	'impls' / 'VulkanShader.cpp',
	'wsi' / 'Common.cpp'
)
engine_inc += include_directories('.')
engine_deps += [vkheaders, vkhpp, vma, vmahpp]
engine_args += ['-DVULKAN_HPP_NO_NODISCARD_WARNINGS', '-DVULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1', '-DVMA_VULKAN_VERSION=1003000', '-DVMA_STATIC_VULKAN_FUNCTIONS=0', 
				'-DVMA_DYNAMIC_VULKAN_FUNCTIONS=1', '-Wno-nullability-completeness']

# Add platform-specific info
if host_machine.system() == 'windows'
    engine_srcs += files('wsi' / 'Win32WSI.cpp')
	engine_deps += [cpp.find_library('gdi32', required: true), cpp.find_library('opengl32', required: true), subproject('glad-wgl', required: true).get_variable('glad_wgl_dep')]
elif host_machine.system() == 'linux'
	if linux_windowing.contains('x11')
		engine_srcs += files('wsi' / 'XWSI.cpp')
	endif
	if linux_windowing.contains('wayland')
		engine_srcs += files('wsi' / 'WaylandWSI.cpp')
	endif
else
    error('Unsupported platform for Vulkan module!')
endif