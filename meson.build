project('cacaoengine', 'cpp', version: 'INDEV', license: 'Apache-2.0', default_options: [ 'cpp_std=c++20' ])

playground = get_option('build_playground')
backend = get_option('use_backend')

if backend == '__DEFAULT__'
	error('A backend must be specified!')
endif

if host_machine.system() == 'windows'
	if meson.get_compiler('c').get_id() == 'msvc'
		add_global_arguments(['/DNOMINMAX','/D _ITERATOR_DEBUG_LEVEL=0','/D_CRT_SECURE_NO_WARNINGS'], language: ['c', 'cpp'])
	else
		add_global_arguments(['-DNOMINMAX','-D_ITERATOR_DEBUG_LEVEL=0','-D_CRT_SECURE_NO_WARNINGS'], language: ['c', 'cpp'])
	endif
endif

subsystem = 'console'
if get_option('windows_noconsole') and host_machine.system() == 'windows'
	add_project_arguments(['-DCACAO_USE_WINMAIN'], language: 'cpp')
	subsystem = 'windows'
endif

glslc = find_program('glslc')

subdir('backends' / backend)

subdir('cacao')

cacao_dep = declare_dependency(include_directories: [
	'cacao/include',
	'libs/spdlog/include',
	'libs/dynalo/include',
	'libs/bsthreadpool',
	'libs/glm',
	'libs/uuid_v4',
	include_directories('cacao')
], dependencies: [
	spv_core,
	spv_c,
	spv_cpp,
	spv_rfl,
	spv_util,
	spv_glsl,
	spv_hlsl,
	spv_msl
], link_with: [
	libbackend,
	libcacao
])

if playground
	subdir('playground')
endif
