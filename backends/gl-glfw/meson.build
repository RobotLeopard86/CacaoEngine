cmake = import('cmake')

glad_dep = subproject('glad', required: true, default_options: [ 'what=gl' ]).get_variable('glad_dep')
stb_dep = subproject('stb', required: true).get_variable('stb_dep')

spv_opts = cmake.subproject_options()
spv_opts.add_cmake_defines({
	'SPIRV_CROSS_STATIC': 'ON',
	'SPIRV_CROSS_SHARED': 'OFF',
	'SPIRV_CROSS_CLI': 'OFF',
	'CMAKE_POSITION_INDEPENDENT_CODE': 'ON'
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
	'GLFW_BUILD_EXAMPLES': 'OFF'
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
	'include',
	'../../libs/spdlog/include',
	'../../libs/thread-pool/include',
	'../../libs/dynalo/include',
	'../../libs/glm',
	'../../libs/uuid_v4',
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
	skyshaders_dep,
	meson.get_compiler('cpp').find_library('atomic', required: true)
]

libbackend = static_library('cacaobackend', include_directories: include_dirs, sources: [
	'src/OpenGL.cpp',
	'src/Window.cpp',
	'src/Texture2D.cpp',
	'src/Cubemap.cpp',
	'src/Shader.cpp',
	'src/Mesh.cpp',
	'src/Skybox.cpp',
	'src/ExceptionCodes.cpp'
], dependencies: backend_deplist)

subdir_done()
