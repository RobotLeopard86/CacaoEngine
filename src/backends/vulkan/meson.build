# Configure Vulkan-Headers
backend_deps += subproject('vulkan-headers', required: true).get_variable('vulkan_headers_dep')

# Configure VulkanMemoryAllocator
backend_deps += subproject('vma', required: true).get_variable('vma_dep')

# Configure Vulkan-Hpp
backend_deps += subproject('vulkan-hpp', required: true).get_variable('vulkan_hpp_dep')

# Configure VulkanMemoryAllocator-Hpp
backend_deps += subproject('vma-hpp', required: true).get_variable('vma_hpp_dep')

# Add sources
src = [
	'Cubemap.cpp',
	'Material.cpp',
	'Mesh.cpp',
	'Shader.cpp',
	'Skybox.cpp',
	'Texture2D.cpp',
	'UIDrawing.cpp',
	'UIView.cpp',
	'VkUtils.cpp',
	'Vulkan.cpp',
	'WindowHooks.cpp'
]
foreach s : src
	backend_sources += 'backends' / 'opengl' / s
endforeach