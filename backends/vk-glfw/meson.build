stb_dep = subproject('stb', required: true).get_variable('stb_dep')

spv_opts = cmake.subproject_options()
spv_opts.add_cmake_defines({
	'SPIRV_CROSS_STATIC': 'ON',
	'SPIRV_CROSS_SHARED': 'OFF',
	'SPIRV_CROSS_CLI': 'OFF',
	'CMAKE_POSITION_INDEPENDENT_CODE': 'ON',
	'CMAKE_BUILD_TYPE': cmake_build_type,
	'CMAKE_MSVC_RUNTIME_LIBRARY': cmake_msvc_lib,
	'CMAKE_POLICY_DEFAULT_CMP0091': 'NEW'
})
spv_sp = cmake.subproject('spirv-cross', options: spv_opts, required: true)
spv_core = spv_sp.dependency('spirv-cross-core')
spv_c = spv_sp.dependency('spirv-cross-c')
spv_cpp = spv_sp.dependency('spirv-cross-cpp')
spv_rfl = spv_sp.dependency('spirv-cross-reflect')
spv_util = spv_sp.dependency('spirv-cross-util')
spv_glsl = spv_sp.dependency('spirv-cross-glsl')
spv_hlsl = spv_sp.dependency('spirv-cross-hlsl')
spv_msl = spv_sp.dependency('spirv-cross-msl')

glfw_opts = cmake.subproject_options()
glfw_opts.add_cmake_defines({
	'BUILD_SHARED_LIBS': 'OFF',
	'GLFW_BUILD_TESTS': 'OFF',
	'GLFW_BUILD_DOCS': 'OFF',
	'GLFW_BUILD_EXAMPLES': 'OFF',
	'CMAKE_BUILD_TYPE': cmake_build_type,
	'CMAKE_MSVC_RUNTIME_LIBRARY': cmake_msvc_lib,
	'CMAKE_POLICY_DEFAULT_CMP0091': 'NEW'
})
if host_machine.system() == 'linux'
	glfw_opts.add_cmake_defines({
		'GLFW_BUILD_WAYLAND': 'ON',
		'GLFW_BUILD_X11': 'ON'
	})
endif
glfw_dep = cmake.subproject('glfw', options: glfw_opts, required: true).dependency('glfw')

vkheaders = subproject('vulkan-headers', required: true).get_variable('vulkan_headers_dep')
vma = subproject('vma', required: true).get_variable('vma_dep')
vkhpp = subproject('vulkan-hpp', required: true).get_variable('vulkan_hpp_dep')
vmahpp = subproject('vma-hpp', required: true).get_variable('vma_hpp_dep')

include_dirs = [
	'../../cacao/include',
	'../common/vk/include',
	'../common/glfw/include',
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
	glfw_dep,
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
	'src/GLFWHooks.cpp',
	'src/VkHooks.cpp',
	'../common/glfw/src/Window.cpp',
	'../common/ExceptionCodes.cpp',
	'../common/vk/src/Vulkan.cpp',
	'../common/vk/src/VkUtils.cpp',
	'../common/vk/src/Cubemap.cpp',
	'../common/vk/src/Texture2D.cpp',
	'../common/vk/src/Mesh.cpp',
	'../common/vk/src/Shader.cpp',
	'../common/vk/src/Skybox.cpp',
	'../common/vk/src/UIView.cpp',
	'../common/vk/src/UIDrawing.cpp',
	'../common/vk/src/Material.cpp',
	'../common/UIShaderGen.cpp'
], dependencies: backend_deplist, cpp_args: ['-DVULKAN_HPP_NO_NODISCARD_WARNINGS', '-DVULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1', '-DVMA_VULKAN_VERSION=1002000', '-DVMA_STATIC_VULKAN_FUNCTIONS=0', '-DVMA_DYNAMIC_VULKAN_FUNCTIONS=1'])

subdir_done()
