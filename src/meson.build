# Get backend
backend = get_option('graphics_backend')

# Configure OpenAL-Soft
alsoft_opts = cmake.subproject_options()
alsoft_opts.add_cmake_defines({
	'BUILD_TESTING': 'OFF',
	'BUILD_SHARED_LIBS': 'OFF',
	'LIBTYPE': 'STATIC',
	'ALSOFT_DLOPEN': 'OFF',
	'ALSOFT_EXAMPLES': 'OFF',
	'ALSOFT_NO_CONFIG_UTIL': 'ON',
	'ALSOFT_EAX': 'OFF',
	'ALSOFT_UTILS': 'OFF',
	'ALSOFT_RTKIT': 'OFF',
	'ALSOFT_EMBED_HRTF_DATA': 'OFF',
	'ALSOFT_BACKEND_WINMM': 'OFF',
	'CMAKE_POSITION_INDEPENDENT_CODE': 'ON',
	'CMAKE_BUILD_TYPE': cmake_build_type,
	'CMAKE_MSVC_RUNTIME_LIBRARY': cmake_msvc_lib,
	'CMAKE_POLICY_DEFAULT_CMP0091': 'NEW'
})
alsoft = cmake.subproject('openal-soft', options: alsoft_opts, required: true).dependency('OpenAL')

# Configure SDL
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
	'SDL_X11': 'ON',
	'SDL_X11_SHARED': 'ON',
	'SDL_OPENGLES': 'OFF',
	'SDL_DIRECTX': 'OFF',
	'SDL_METAL': 'OFF',
	'SDL_OPENGL': backend == 'opengl' ? 'ON' : 'OFF',
	'SDL_VULKAN': backend == 'vulkan' ? 'ON' : 'OFF',
	'CMAKE_BUILD_TYPE': cmake_build_type,
	'CMAKE_MSVC_RUNTIME_LIBRARY': cmake_msvc_lib,
	'CMAKE_POLICY_DEFAULT_CMP0091': 'NEW'
})
sdl_opts.append_compile_args('c', '-Dhid_darwin_set_open_exclusive(x)=;')
sdl3_sp = cmake.subproject('sdl3', options: sdl_opts, required: true)
sdl3 = sdl3_sp.dependency('SDL3-static')

# Configure SPIRV-Cross
spv_cross_opts = cmake.subproject_options()
spv_cross_opts.add_cmake_defines({
	'SPIRV_CROSS_STATIC': 'ON',
	'SPIRV_CROSS_SHARED': 'OFF',
	'SPIRV_CROSS_CLI': 'OFF',
	'CMAKE_POSITION_INDEPENDENT_CODE': 'ON',
	'CMAKE_BUILD_TYPE': cmake_build_type,
	'CMAKE_MSVC_RUNTIME_LIBRARY': cmake_msvc_lib,
	'CMAKE_POLICY_DEFAULT_CMP0091': 'NEW'
})
spv_cross_sp = cmake.subproject('spirv-cross', options: spv_cross_opts, required: true)
spv_core = spv_cross_sp.dependency('spirv-cross-core')
spv_c = spv_cross_sp.dependency('spirv-cross-c')
spv_cpp = spv_cross_sp.dependency('spirv-cross-cpp')
spv_rfl = spv_cross_sp.dependency('spirv-cross-reflect')
spv_util = spv_cross_sp.dependency('spirv-cross-util')
spv_glsl = spv_cross_sp.dependency('spirv-cross-glsl')
spv_hlsl = spv_cross_sp.dependency('spirv-cross-hlsl')
spv_msl = spv_cross_sp.dependency('spirv-cross-msl')
spirv_cross = declare_dependency(dependencies: [spv_core, spv_c, spv_cpp, spv_rfl, spv_util, spv_glsl, spv_hlsl, spv_msl]) # This is just a utility