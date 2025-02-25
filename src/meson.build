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
spirv_cross = declare_dependency(dependencies: [spv_core, spv_c, spv_cpp, spv_rfl, spv_util, spv_glsl, spv_hlsl, spv_msl])

# Configure crossguid
guid_opts = cmake.subproject_options()
guid_opts.add_cmake_defines({
	'BUILD_SHARED_LIBS': 'OFF',
	'CMAKE_POSITION_INDEPENDENT_CODE': 'ON',
	'CROSSGUID_TESTS': 'OFF',
	'CMAKE_BUILD_TYPE': cmake_build_type,
	'CMAKE_MSVC_RUNTIME_LIBRARY': cmake_msvc_lib,
	'CMAKE_POLICY_DEFAULT_CMP0091': 'NEW'
})
if meson.get_compiler('cpp').get_id() == 'msvc'
	guid_opts.append_compile_args('cpp', '/FI', 'stdint.h')
else
	guid_opts.append_compile_args('cpp', '-include', 'stdint.h')
endif
guid = cmake.subproject('guid', options: guid_opts, required: true).dependency('crossguid')

# Configure ICU
icu_sp = subproject('icu', required: true, default_options: ['default_library=static'])
icu = icu_sp.get_variable('icuuc_dep')

# Configure FreeType
freetype_sp = subproject('freetype', required: true, default_options: ['default_library=static', 'png=disabled', 'zlib=disabled', 'brotli=disabled', 'bzip2=disabled', 'harfbuzz=disabled'])
freetype = freetype_sp.get_variable('freetype_dep')

# Configure Harfbuzz
harfbuzz_sp = subproject('harfbuzz', required: true, default_options: ['default_library=static', 'freetype=enabled', 'cairo=disabled', 'graphite2=disabled', 'glib=disabled', 'gobject=disabled', 'chafa=disabled', 'icu=enabled', 'gdi=disabled', 'directwrite=disabled', 'coretext=disabled', 'wasm=disabled', 'tests=disabled', 'introspection=disabled', 'docs=disabled', 'utilities=disabled'])
harfbuzz = harfbuzz_sp.get_variable('libharfbuzz_dep')
harfbuzz_icu = harfbuzz_sp.get_variable('libharfbuzz_icu_dep')
harfbuzz_sub = harfbuzz_sp.get_variable('libharfbuzz_subset_dep')

# Configure spdlog, thread-pool, dynalo, and GLM
spdlog = subproject('spdlog', required: true).get_variable('spdlog_dep')
thread_pool = subproject('thread-pool', required: true).get_variable('thread_pool_dep')
dynalo = subproject('dynalo', required: true).get_variable('dynalo_dep')
glm = subproject('glm', required: true).get_variable('glm_dep')

# Find GLSLC
glslc = find_program('glslc', required: true)

# Generate shaders
shader_sources = [
	'image', 'text', 'uiquad', 'skybox'
]
shader_outputs = []
foreach shader : shader_sources
	shader_outputs += custom_target('coreshaders_' + shader + '_v', output: shader + '.vert.txt', input: shader + '.vert', command: [glslc, '-mfmt=c', '-w', '--target-env=vulkan1.3', '-O', '@INPUT@', '-o', '@OUTPUT@'])
	shader_outputs += custom_target('coreshaders_' + shader + '_f', output: shader + '.frag.txt', input: shader + '.frag', command: [glslc, '-mfmt=c', '-w', '--target-env=vulkan1.3', '-O', '@INPUT@', '-o', '@OUTPUT@'])
endforeach
coreshaders = declare_dependency(sources: shader_outputs)

# Get Windows subsystem
subsystem = 'console'
if get_option('windows_noconsole')
	subsystem = 'windows'
endif

# Check if we need to link with Pipewire
need_pipewire = run_command([ python, meson.project_source_root() / 'scripts' / 'openal_checkpipewire.py'], check: false).returncode()
if need_pipewire == 0
	# We have to make sure that pkg-config is available to find Pipewire
	find_program('pkg-config', required: true)
endif

# Define variables for backends to fill in
backend_sources = []
backend_deps = []

# Configure backend
subdir('backends' / backend)

# Grab C++ compiler for library search
cxx = meson.get_compiler('cpp')

# Set Windows-specific compiler args
args = []
link_args = []
extra_libs = []
if host_machine.system() == 'windows'
	if subsystem == 'windows'
		link_args += '/ENTRY:mainCRTStartup'
	endif
	link_args += '/NODEFAULTLIB'
	libs = [
		'msvcrt',
		'ucrt',
		'msvcprt',
		'vcruntime'
	]
	foreach lib : libs
		extra_libs += cxx.find_library(bt == 'debug' ? lib + 'd' : lib, required: true)
	endforeach
endif


# Make executable
executable(get_option('exe_name'), include_directories: [
	'../include',
	'core',
	'common',
	'backends' / backend
], dependencies: [
	backend_deps,
	extra_libs,
	glm,
	yaml_cpp,
	alsoft,
	sdl3,
	spirv_cross,
	guid,
	icu,
	freetype,
	harfbuzz,
	harfbuzz_sp,
	harfbuzz_icu,
	spdlog,
	thread_pool,
	dynalo,
	coreshaders,
	cxx.find_library('imm32', required: host_machine.system() == 'windows'),
	cxx.find_library('winmm', required: host_machine.system() == 'windows'),
	cxx.find_library('setupapi', required: host_machine.system() == 'windows'),
	cxx.find_library('version', required: host_machine.system() == 'windows'),
	cxx.find_library('atomic', required: host_machine.system() == 'linux'),
	cxx.find_library('dl', static: true, required: host_machine.system() == 'linux'),
	dependency('libpipewire-0.3', method: 'pkg-config', required: host_machine.system() == 'linux' and need_pipewire == 0),
	dependency('appleframeworks', modules: ['IOKit', 'AudioToolbox', 'CoreFoundation', 'Cocoa', 'CoreAudio', 'CoreVideo', 'Carbon'], required: host_machine.system() == 'darwin')
], sources: [
	backend_sources,
	'core/Assert.cpp',
	'core/AssetManager.cpp',
	'core/AudioPlayer.cpp',
	'core/AudioSystem.cpp',
	'core/CommonAssetLoader.cpp',
	'core/DynTickController.cpp',
	'core/Engine.cpp',
	'core/EventManager.cpp',
	'core/Font.cpp',
	'core/FreetypeOwner.cpp',
	'core/Input.cpp',
	'core/Log.cpp',
	'core/Model.cpp',
	'core/PerspectiveCamera.cpp',
	'core/Renderables.cpp',
	'core/RenderController.cpp',
	'core/Screen.cpp',
	'core/Sound.cpp',
	'core/Startup.cpp',
	'core/Texture2D.cpp',
	'core/Transform.cpp',
	'core/UIView.cpp',
	'core/Window.cpp',
	'core/WorldManager.cpp'
], export_dynamic: true, pie: true, win_subsystem: subsystem, cpp_args: args, link_args: link_args)