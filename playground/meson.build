glslc = find_program('glslc')

shaders = [
	'src/shaders/color.vert',
	'src/shaders/color.frag'
]
shader_targets = []

foreach shader : shaders
	split = shader.split('/')
	shader_targets += custom_target('shader @0@'.format(split[-1]),
		command: [glslc, '@INPUT@', '-o', '@OUTPUT@'],
		input: shader,
		output: '@PLAINNAME@.spv',
		build_by_default: true
	)
endforeach

launch_module = shared_module('launch', name_prefix: '', dependencies: cacao_dep, sources: 'src/Playground.cpp', build_by_default: true)

run_target('bundle_playground', command: ['python', meson.project_source_root()/'playground'/'bundle.py', meson.project_build_root(), backend], depends: [
	launch_module
])

run_target('run_playground', command: ['python', meson.project_source_root()/'playground'/'bundleandrun.py', meson.project_build_root(), backend], depends: [
	launch_module
])

subdir_done()