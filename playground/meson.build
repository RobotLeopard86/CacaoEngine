glslc_spv_cmd = [ glslc, '-mfmt=bin', '-o', '@OUTPUT@', '@INPUT@']
colorvert = custom_target('color.vert', command: glslc_spv_cmd, input: 'src/shaders/color.vert', output: '@PLAINNAME@.spv', build_always_stale: true, build_by_default: true)
colorfrag = custom_target('color.frag', command: glslc_spv_cmd, input: 'src/shaders/color.frag', output: '@PLAINNAME@.spv', build_always_stale: true, build_by_default: true)

launch_module = shared_module('launch', name_prefix: '', dependencies: cacao_dep, sources: [
	'src/Playground.cpp',
	colorvert,
	colorfrag
], build_by_default: true)

run_target('bundle_playground', command: [python, meson.global_source_root()/'scripts'/'bundle_playground.py'], depends: [
	launch_module,
	cacao_exe
])

run_target('run_playground', command: [python, meson.global_source_root()/'scripts'/'playground_bundleandrun.py'], depends: [
	launch_module,
	cacao_exe
])

subdir_done()