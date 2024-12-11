project('cacaoengine', 'cpp', 'c', meson_version: '>=1.1', version: 'INDEV', license: 'Apache-2.0', default_options: [ 'cpp_std=c++20', 'b_vscrt=from_buildtype' ])

playground = get_option('build_playground')
backend = get_option('use_backend')

if backend == '__DEFAULT__'
	error('A backend must be specified!')
endif

python = find_program('python3')

if get_option('backend') == 'xcode'
	warning('Generation of Xcode projects is not supported by Cacao Engine yet, so things may or may not work! Continue at your own risk!')
	run_command([ python, 'scripts/delay.py' ], check: false)
endif

if get_option('backend').contains('vs')
	error('Meson\'s Visual Studio generator does not function in a way that allows Cacao Engine to function properly. Instead, follow the instructions in "VISUALSTUDIO.md" to get it working.')
endif

cc = meson.get_compiler('cpp')
if host_machine.system() == 'windows'
	if cc.get_id() != 'msvc' or cc.get_linker_id() != 'link'
		warning('You are not using the MSVC toolchain. This is the only supported Windows toolchain, so things may or may not work! Continue at your own risk!')
		run_command([ python, 'scripts/delay.py' ], check: false)
	endif
elif host_machine.system() == 'linux'
	if cc.get_id() != 'clang' or cc.get_linker_id() != 'ld.lld'
		warning('You are not using Clang with LLD. This is the only supported Linux toolchain, so things may or may not work! Continue at your own risk!')
		run_command([ python, 'scripts/delay.py' ], check: false)
	endif
elif host_machine.system() == 'darwin'
	if cc.get_id() != 'clang' or cc.get_linker_id() != 'ld64.lld'
		warning('You are not using Clang with LLD. This is the only supported macOS toolchain, so things may or may not work! Continue at your own risk!')
		run_command([ python, 'scripts/delay.py' ], check: false)
	endif
else
	warning('Your system is not a supported platform and things may or may not work! Continue at your own risk!')
	run_command([ python, 'scripts/delay.py' ], check: false)
endif

bt = get_option('buildtype')

if bt == 'debug'
	if meson.get_compiler('c').get_id() == 'msvc'
		add_project_arguments(['/D_ITERATOR_DEBUG_LEVEL=2'], language: ['c', 'cpp'])
	else
		add_project_arguments(['-D_ITERATOR_DEBUG_LEVEL=2'], language: ['c', 'cpp'])
	endif
endif

if meson.get_compiler('c').get_id() != 'msvc'
	add_project_arguments(['-Wno-nullability-completeness'], language: ['c', 'cpp'])
endif

subsystem = 'console'
if get_option('windows_noconsole') and host_machine.system() == 'windows'
	subsystem = 'windows'
endif

compiler_args = []

if host_machine.system() == 'darwin'
	add_project_dependencies(dependency('appleframeworks', modules: ['IOKit', 'AudioToolbox', 'CoreFoundation', 'Cocoa', 'CoreAudio', 'CoreVideo', 'Carbon'], required: true), language: ['c', 'cpp'])
	add_project_arguments('-fexperimental-library', language: 'cpp')
	add_project_link_arguments('-fexperimental-library', language: 'cpp')
	compiler_args += '-fexperimental-library'
endif

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

glslc = find_program('glslc')

glslc_text_gen = generator(glslc, arguments: ['-w', '-mfmt=c', '-o', '@OUTPUT@', '@INPUT@'], output: '@PLAINNAME@.txt')
core_shaders_s = glslc_text_gen.process([
	'cacao/shaders/skybox.vert', 
	'cacao/shaders/skybox.frag',
	'cacao/shaders/uiquad.vert', 
	'cacao/shaders/uiquad.frag',
	'cacao/shaders/text.vert',
	'cacao/shaders/text.frag',
	'cacao/shaders/image.vert',
	'cacao/shaders/image.frag'
])

core_shaders = declare_dependency(sources: [
	core_shaders_s
])

icu_sp = subproject('icu', required: true, default_options: ['default_library=static'])
icu = icu_sp.get_variable('icuuc_dep')

freetype_sp = subproject('freetype', required: true, default_options: ['default_library=static', 'png=disabled', 'zlib=disabled', 'brotli=disabled', 'bzip2=disabled', 'harfbuzz=disabled'])
freetype = freetype_sp.get_variable('freetype_dep')

harfbuzz_sp = subproject('harfbuzz', required: true, default_options: ['default_library=static', 'freetype=enabled', 'cairo=disabled', 'graphite2=disabled', 'glib=disabled', 'gobject=disabled', 'chafa=disabled', 'icu=enabled', 'gdi=disabled', 'directwrite=disabled', 'coretext=disabled', 'wasm=disabled', 'tests=disabled', 'introspection=disabled', 'docs=disabled', 'utilities=disabled'])
harfbuzz = harfbuzz_sp.get_variable('libharfbuzz_dep')
harfbuzz_icu = harfbuzz_sp.get_variable('libharfbuzz_icu_dep')
harfbuzz_sub = harfbuzz_sp.get_variable('libharfbuzz_subset_dep')

glm_dep = subproject('glm', required: true).get_variable('glm_dep')
spdlog_dep = subproject('spdlog', required: true).get_variable('spdlog_dep')
dynalo_dep = subproject('dynalo', required: true).get_variable('dynalo_dep')
thread_pool_dep = subproject('thread-pool', required: true).get_variable('thread_pool_dep')
dr_libs_dep = subproject('dr_libs', required: true).get_variable('dr_libs_dep')

cmake = import('cmake')

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
yaml_dep = cmake.subproject('yaml', options: yaml_opts, required: true).dependency('yaml-cpp')

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
alsoft_dep = cmake.subproject('openal-soft', options: alsoft_opts, required: true).dependency('OpenAL')

if bt.contains('debug')
	add_project_arguments(['-DDEBUG'], language: 'cpp')
endif

subdir('backends' / backend)
subdir('cacao')

cacao_dep = declare_dependency(include_directories: [
	'cacao/include',
], dependencies: [
	yaml_dep,
	assimp_dep,
	zlib_dep,
	libdl,
	spv_core,
	spv_c,
	spv_cpp,
	spv_util,
	spv_rfl,
	spv_glsl,
	spv_hlsl,
	spv_msl,
	core_shaders,
	alsoft_dep,
	ogg,
	vorbis,
	vorbisfile,
	opus,
	opusfile,
	guid_dep,
	freetype,
	icu,
	harfbuzz,
	harfbuzz_icu,
	harfbuzz_sub,
	glm_dep,
	dr_libs_dep,
	dynalo_dep,
	spdlog_dep,
	thread_pool_dep
], link_with: cacao_exe, compile_args: compiler_args, link_args: compiler_args)

if playground
	subdir('playground')
endif
