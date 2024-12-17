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

sdl_opts = cmake.subproject_options()
sdl_opts.add_cmake_defines({
	'SDL_TESTS': 'OFF',
	'SDL_SHARED': 'OFF',
	'SDL_STATIC': 'ON',
	'SDL_STATIC_PIC': 'ON',
	'SDL_TEST_LIBRARY': 'OFF',
	'SDL_AUDIO': 'OFF',
	'SDL_VIDEO': 'ON',
	'SDL_RENDER': 'OFF',
	'SDL_CAMERA': 'OFF',
	'SDL_JOYSTICK': 'OFF',
	'SDL_HAPTIC': 'OFF',
	'SDL_HIDAPI': 'OFF',
	'SDL_POWER': 'OFF',
	'SDL_SENSOR': 'OFF',
	'SDL_DIALOG': 'OFF',
	'CMAKE_POSITION_INDEPENDENT_CODE': 'ON',
	'SDL_WAYLAND': 'ON',
	'SDL_WAYLAND_SHARED': 'ON',
	'SDL_WAYLAND_LIBDECOR': 'ON',
	'SDL_WAYLAND_LIBDECOR_SHARED': 'ON',
	'SDL_METAL': 'OFF',
	'SDL_VULKAN': 'OFF',
	'SDL_X11': 'ON',
	'SDL_X11_SHARED': 'ON',
	'CMAKE_BUILD_TYPE': cmake_build_type,
	'CMAKE_MSVC_RUNTIME_LIBRARY': cmake_msvc_lib,
	'CMAKE_POLICY_DEFAULT_CMP0091': 'NEW'
})
sdl_opts.append_compile_args('c', '-Dhid_darwin_set_open_exclusive(x)=;')
sdl3_sp = cmake.subproject('sdl3', options: sdl_opts, required: true)
sdl3 = sdl3_sp.dependency('SDL3-static')

include_dirs = [
	'../../cacao/include',
	'../common/gl/include',
	'../common/sdl/include',
	'../common',
	include_directories('../../cacao'),
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
	thread_pool_dep,
	yaml_dep,
	alsoft_dep
]

if host_machine.system() == 'windows'
	backend_deplist += meson.get_compiler('cpp').find_library('imm32', required: true)
endif

libbackend = static_library(
	'cacaobackend',
	include_directories: include_dirs,
	sources: [
		'src/SDLHooks.cpp',
		'../common/sdl/src/Window.cpp',
		'../common/gl/src/Texture2D.cpp',
		'../common/gl/src/Cubemap.cpp',
		'../common/gl/src/Shader.cpp',
		'../common/gl/src/Mesh.cpp',
		'../common/gl/src/Skybox.cpp',
		'../common/gl/src/OpenGL.cpp',
		'../common/gl/src/UIDrawing.cpp',
		'../common/gl/src/UIView.cpp',
		'../common/gl/src/Material.cpp',
		'../common/UIShaderGen.cpp',
		'../common/ExceptionCodes.cpp',
	],
	dependencies: backend_deplist,
)

subdir_done()