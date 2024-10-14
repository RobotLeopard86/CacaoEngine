glad_dep = subproject('glad', required: true).get_variable('glad_dep')
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

include_dirs = [
	'../../cacao/include',
	'../common/gl/include',
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
	glad_dep,
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
	thread_pool_dep
]

if host_machine.system() == 'windows'
	backend_deplist += meson.get_compiler('cpp').find_library('imm32', required: true)
endif

libbackend = static_library('cacaobackend', include_directories: include_dirs, sources: [
	'src/GLFWHooks.cpp',
	'../common/glfw/src/Window.cpp',
	'../common/gl/src/Texture2D.cpp',
	'../common/gl/src/Cubemap.cpp',
	'../common/gl/src/Shader.cpp',
	'../common/gl/src/Mesh.cpp',
	'../common/gl/src/Skybox.cpp',
	'../common/gl/src/OpenGL.cpp',
	'../common/gl/src/UIDrawing.cpp',
	'../common/gl/src/UIView.cpp',
	'../common/UIShaderGen.cpp',
	'../common/ExceptionCodes.cpp'
], dependencies: backend_deplist)

subdir_done()
