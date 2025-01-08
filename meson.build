project('cacaoengine', 'cpp', 'c', meson_version: '>=1.1', version: 'INDEV', license: 'Apache-2.0', default_options: [ 'cpp_std=c++20', 'b_vscrt=from_buildtype' ])

# We are Ninja-only
if get_option('backend') != 'ninja'
	error('Only the Ninja backend is supported. If you\'re trying to use Visual Studio, see the Visual Studio page in the documentation for instructions.')
endif

# Find Python and GLSLC
python = find_program('python3', required: true)
glslc = find_program('glslc', required: true)

# Warn the user if they are trying to use an unsupported setup
compiler = meson.get_compiler('cpp')
if compiler.get_id() != 'clang' or not compiler.get_linker_id().contains('lld')
	fs = import('fs')
	okfile = meson.project_build_root() / 'badconfig_ok'
	if not fs.exists(okfile)
		res = run_command([python, meson.project_source_root() / 'scripts' / 'badsetup_confirm', okfile], check: false).returncode()
		assert(res == 0, 'User opted to not proceed with unsupported configuration')
	endif
endif

# Get some options
bt = get_option('buildtype')
comps = get_option('components')
backend = get_option('graphics_backend')

# Process components to deal with dependencies
processed_comps = comps
if processed_comps.contains('demo') and not processed_comps.contains('crt')
	processed_comps += 'crt'
endif
if processed_comps.contains('crt')
	if not processed_comps.contains('core')
		processed_comps += 'core'
	endif
	if not processed_comps.contains('crt-formats')
		processed_comps += 'crt-formats'
	endif
endif

# Generate some variables necessary for CMake to work correctly
cmake_build_type = ''
cmake_msvc_lib = 'MultiThreaded'
if bt.contains('debug')
	if bt == 'debug'
		cmake_build_type = 'Debug'
		cmake_msvc_lib += 'Debug'
	else
		cmake_build_type = 'RelWithDebInfo'
	endif
elif bt == 'plain' or bt == 'minsize'
	cmake_build_type = 'MinSizeRel'
elif bt == 'release'
	cmake_build_type = 'Release'
else
	error('Unsupported build type!')
endif
cmake_msvc_lib += 'DLL'

# Import CMake
cmake = import('cmake')

# Set some Windows-specific options
if host_machine.system() == 'windows'
	subsystem = 'console'
	if get_option('windows_noconsole')
		subsystem = 'windows'
	endif
	add_project_arguments(['-D_MT', '-D_DLL'], language: ['c', 'cpp'])
	add_project_link_arguments(['-Xlinker', '/NODEFAULTLIB'], language: ['c', 'cpp'])
	if bt == 'debug'
		add_project_arguments(['-D_ITERATOR_DEBUG_LEVEL=2'], language: ['c', 'cpp'])
		add_project_link_arguments(['-lmsvcrtd', '-lucrtd', '-lvcruntimed'], language: ['c', 'cpp'])
		add_project_link_arguments(['-lmsvcprtd'], language: ['cpp'])
	else
		add_project_link_arguments(['-lmsvcrt', '-lucrt', '-lvcruntime'], language: ['c', 'cpp'])
		add_project_link_arguments(['-lmsvcprt'], language: ['cpp'])
	endif
endif

# Set some macOS-specific options
dep_compiler_args = []
if host_machine.system() == 'darwin'
	add_project_dependencies(dependency('appleframeworks', modules: ['IOKit', 'AudioToolbox', 'CoreFoundation', 'Cocoa', 'CoreAudio', 'CoreVideo', 'Carbon'], required: true), language: ['c', 'cpp'])
	add_project_arguments('-fexperimental-library', language: 'cpp')
	add_project_link_arguments('-fexperimental-library', language: 'cpp')
	dep_compiler_args += '-fexperimental-library'
endif

# Define debug macro
if bt.contains('debug')
	add_project_arguments(['-D_DEBUG'], language: 'cpp')
endif

# Get some shared dependencies
icu_sp = subproject('icu', required: true, default_options: ['default_library=static'])
icu = icu_sp.get_variable('icuuc_dep')

freetype_sp = subproject('freetype', required: true, default_options: ['default_library=static', 'png=disabled', 'zlib=disabled', 'brotli=disabled', 'bzip2=disabled', 'harfbuzz=disabled'])
freetype = freetype_sp.get_variable('freetype_dep')

harfbuzz_sp = subproject('harfbuzz', required: true, default_options: ['default_library=static', 'freetype=enabled', 'cairo=disabled', 'graphite2=disabled', 'glib=disabled', 'gobject=disabled', 'chafa=disabled', 'icu=enabled', 'gdi=disabled', 'directwrite=disabled', 'coretext=disabled', 'wasm=disabled', 'tests=disabled', 'introspection=disabled', 'docs=disabled', 'utilities=disabled'])
harfbuzz = harfbuzz_sp.get_variable('libharfbuzz_dep')
harfbuzz_icu = harfbuzz_sp.get_variable('libharfbuzz_icu_dep')
harfbuzz_sub = harfbuzz_sp.get_variable('libharfbuzz_subset_dep')

glm = subproject('glm', required: true).get_variable('glm_dep')
spdlog = subproject('spdlog', required: true).get_variable('spdlog_dep')
thread_pool = subproject('thread-pool', required: true).get_variable('thread_pool_dep')
dr_libs = subproject('dr_libs', required: true).get_variable('dr_libs_dep')

yaml_opts = cmake.subproject_options()
yaml_opts.add_cmake_defines({
	'BUILD_SHARED_LIBS': 'OFF',
	'CMAKE_INSTALL_PREFIX': meson.project_build_root() / '.cmakeinstall',
	'YAML_CPP_BUILD_TOOLS': 'OFF',
	'YAML_CPP_BUILD_TESTS': 'OFF',
	'CMAKE_POSITION_INDEPENDENT_CODE': 'ON',
	'CMAKE_BUILD_TYPE': cmake_build_type,
	'CMAKE_MSVC_RUNTIME_LIBRARY': cmake_msvc_lib,
	'CMAKE_POLICY_DEFAULT_CMP0091': 'NEW'
})
yaml_opts.set_install(true)
yaml = cmake.subproject('yaml', options: yaml_opts, required: true).dependency('yaml-cpp')

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

# Components
if processed_comps.contains('core') 
	subdir('core')
	subdir('backends' / backend)
endif
if processed_comps.contains('crt-formats')
        subdir('crt-formats')
endif
if processed_comps.contains('crt-unpacked-formats')
        subdir('crt-unpacked-formats')
endif
if processed_comps.contains('core')
        subdir('core')
endif
if processed_comps.contains('crt')
        subdir('crt')
endif
if processed_comps.contains('demo')
        subdir('demo')
endif
