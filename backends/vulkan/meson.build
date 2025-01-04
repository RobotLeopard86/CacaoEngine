stb_dep = subproject('stb', required: true).get_variable('stb_dep')

vkheaders = subproject('vulkan-headers', required: true).get_variable('vulkan_headers_dep')
vma = subproject('vma', required: true).get_variable('vma_dep')
vkhpp = subproject('vulkan-hpp', required: true).get_variable('vulkan_hpp_dep')
vmahpp = subproject('vma-hpp', required: true).get_variable('vma_hpp_dep')

include_dirs = [
	'../../cacao/include',
	'include',
	'../common',
	include_directories('../../cacao')
]

backend_deplist = [
	spv_core,
	spv_c,
	spv_cpp,
	spv_util,
	spv_rfl,
	spv_glsl,
	spv_hlsl,
	spv_msl,
	sdl3,
	stb_dep,
	freetype,
	icu,
	harfbuzz,
	harfbuzz_icu,
	harfbuzz_sub,
	core_shaders,
	glm_dep,
	dr_libs_dep,
	dynalo_dep,
	spdlog_dep,
	thread_pool_dep,
	alsoft_dep,
	vkheaders,
	vkhpp,
	vma,
	vmahpp,
	yaml_dep
]

if host_machine.system() == 'windows'
	backend_deplist += meson.get_compiler('cpp').find_library('imm32', required: true)
endif

libbackend = static_library('cacaobackend', include_directories: include_dirs, sources: [
	'src/WindowHooks.cpp',
	'src/Vulkan.cpp',
	'src/VkUtils.cpp',
	'src/Cubemap.cpp',
	'src/Texture2D.cpp',
	'src/Mesh.cpp',
	'src/Shader.cpp',
	'src/Skybox.cpp',
	'src/UIView.cpp',
	'src/UIDrawing.cpp',
	'src/Material.cpp',
	'../common/UIShaderGen.cpp'
], dependencies: backend_deplist, cpp_args: ['-DVULKAN_HPP_NO_NODISCARD_WARNINGS', '-DVULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1', '-DVMA_VULKAN_VERSION=1003000', '-DVMA_STATIC_VULKAN_FUNCTIONS=0', '-DVMA_DYNAMIC_VULKAN_FUNCTIONS=1'])

subdir_done()
