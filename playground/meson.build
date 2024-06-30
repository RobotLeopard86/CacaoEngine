spv_gen = generator(glslc, output: '@PLAINNAME@.spv', arguments: ['-mfmt=bin', '@INPUT@', '-o', '@OUTPUT@'])

shaders = spv_gen.process('src/shaders/ico.vert', 'src/shaders/ico.frag', 'src/shaders/prism.vert', 'src/shaders/prism.frag')

launch_module = shared_module('launch', name_prefix: '', dependencies: cacao_dep, sources: [
	'src/Playground.cpp',
	shaders
], build_by_default: true)

run_target('bundle_playground', command: [python, meson.project_source_root()/'scripts'/'bundle_playground.py'], depends: [
	launch_module,
	cacao_exe
])

run_target('run_playground', command: [python, meson.project_source_root()/'scripts'/'playground_bundleandrun.py'], depends: [
	launch_module,
	cacao_exe
])

subdir_done()