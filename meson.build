project('cacaoengine', 'cpp', 'c', version: 'INDEV', license: 'Apache-2.0', default_options: [ 'cpp_std=c++20', 'b_vscrt=from_buildtype' ])

playground = get_option('build_playground')
backend = get_option('use_backend')

if backend == '__DEFAULT__'
	error('A backend must be specified!')
endif

python = find_program('python')

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

if host_machine.system() == 'windows'
	if meson.get_compiler('c').get_id() == 'msvc'
		add_global_arguments(['/DNOMINMAX', '/D_CRT_SECURE_NO_WARNINGS', '/D_ITERATOR_DEBUG_LEVEL=0'], language: ['c', 'cpp'])
	else
		add_global_arguments(['-DNOMINMAX', '-D_CRT_SECURE_NO_WARNINGS', '-D_ITERATOR_DEBUG_LEVEL=0'], language: ['c', 'cpp'])
	endif
endif

subsystem = 'console'
if get_option('windows_noconsole') and host_machine.system() == 'windows'
	subsystem = 'windows'
endif

bt = get_option('buildtype')
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

glslc_text_gen = generator(glslc, arguments: ['-w', '-mfmt=c', '-o', '@OUTPUT@', '@INPUT@'])
core_shaders_s = glslc_text_gen.process([
	'cacao/shaders/skybox.vert', 
	'cacao/shaders/skybox.frag',
	'cacao/shaders/uiquad.vert', 
	'cacao/shaders/uiquad.frag'
])

core_shaders = declare_dependency(sources: [
	core_shaders_s
])

python = find_program('python')

subdir('backends' / backend)
subdir('cacao')

cacao_dep = declare_dependency(include_directories: [
	'cacao/include',
	'libs/spdlog/include',
	'libs/thread-pool/include',
	'libs/dynalo/include',
	'libs/glm',
	include_directories('cacao')
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
	harfbuzz_sub
], link_with: cacao_exe)

if playground
	subdir('playground')
endif
