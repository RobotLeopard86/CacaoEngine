project('cacaoengine', 'cpp', version: 'INDEV', license: 'Apache-2.0', default_options: [ 'cpp_std=c++20' ])

playground = get_option('build_playground')
backend = get_option('use_backend')

if backend == '__DEFAULT__'
	error('A backend must be specified!')
endif
subdir('backends' / backend)

subdir('cacao')

cacao_dep = declare_dependency(include_directories: [
	'cacao/include',
	'libs/spdlog/include',
	'libs/dynalo/include',
	'libs/bsthreadpool',
	'libs/glm',
	'libs/uuid_v4'
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