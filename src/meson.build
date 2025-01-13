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

# Configure Assimp
assimp_opts = cmake.subproject_options()
assimp_opts.add_cmake_defines({
	'BUILD_SHARED_LIBS': 'OFF',
	'CMAKE_INSTALL_PREFIX': meson.project_build_root() / '.cmakeinstall',
	'ASSIMP_BUILD_ZLIB': 'ON',
	'ASSIMP_NO_EXPORT': 'ON',
	'ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT': 'ON',
	'ASSIMP_BUILD_SAMPLES': 'OFF',
	'ASSIMP_BUILD_TESTS': 'OFF',
	'CMAKE_BUILD_TYPE': cmake_build_type,
	'CMAKE_MSVC_RUNTIME_LIBRARY': cmake_msvc_lib,
	'CMAKE_POLICY_DEFAULT_CMP0091': 'NEW'
})
assimp_sp = cmake.subproject('assimp', options: assimp_opts, required: true)
assimp = assimp_sp.dependency('assimp')
zlib = assimp_sp.dependency('zlibstatic')

# Configure audio libraries
audio_opts = cmake.subproject_options()
audio_opts.add_cmake_defines({
    'BUILD_SHARED_LIBS': 'OFF',
    'OP_DISABLE_HTTP': 'ON',
   	'OP_DISABLE_EXAMPLES': 'ON',
    'OP_DISABLE_DOCS': 'ON',
    'OPUS_BUILD_SHARED_LIBRARY': 'OFF',
    'OPUS_BUILD_TESTING': 'OFF',
    'OPUS_CUSTOM_MODES': 'ON',
    'OPUS_BUILD_PROGRAMS': 'OFF',
	'OPUS_STATIC_RUNTIME': 'OFF',
    'CMAKE_POSITION_INDEPENDENT_CODE': 'ON',
	'CMAKE_BUILD_TYPE': cmake_build_type,
	'CMAKE_MSVC_RUNTIME_LIBRARY': cmake_msvc_lib,
	'CMAKE_POLICY_DEFAULT_CMP0091': 'NEW'
})
audio_sp = cmake.subproject('audiolibs', options: audio_opts, required: true)
ogg = audio_sp.dependency('ogg')
vorbis = audio_sp.dependency('vorbis')
vorbisfile = audio_sp.dependency('vorbisfile')
opus = audio_sp.dependency('opus')
opusfile = audio_sp.dependency('opusfile')
audio_libs = declare_dependency(dependencies: [ogg, vorbis, vorbisfile, opus, opusfile])

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

# Configure spdlog, thread-pool, dynalo, and dr_libs
spdlog = subproject('spdlog', required: true).get_variable('spdlog_dep')
thread_pool = subproject('thread-pool', required: true).get_variable('thread_pool_dep')
dynalo = subproject('dynalo', required: true).get_variable('dynalo_dep')
dr_libs = subproject('dr_libs', required: true).get_variable('dr_libs_dep')

# Get backend
backend = get_option('graphics_backend')

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
	find_program('pkg-config', required: true)
	exe_deps += 
endif

# Define variables for backends to fill in
backend_sources = []
backend_deps = []

# Configure backend
subdir('backends' / backend)

# Grab C++ compiler for library search
cxx = meson.get_compiler('cpp')

# Make executable
executable(get_option('exe_name'), include_directories: [
	'../include',
	'core',
	'common',
	'backends' / backend
], dependencies: [
	backend_deps,
	glm,
	yaml_cpp,
	alsoft,
	sdl3,
	spirv_cross,
	assimp,
	zlib,
	audio_libs,
	guid,
	icu,
	freetype,
	harfbuzz,
	harfbuzz_sp,
	harfbuzz_icu,
	spdlog,
	thread_pool,
	dynalo,
	dr_libs,
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
	backend_sources
], export_dynamic: true, pie: true, win_subsystem: subsystem)