glslc_spv_cmd = [ glslc, '-mfmt=bin', '-o', '@OUTPUT@', '@INPUT@']
custom_target('playground_shaders:color.vert', command: glslc_spv_cmd, input: 'src/shaders/color.vert', output: '@PLAINNAME@.spv', build_always_stale: true, build_by_default: true)
custom_target('playground_shaders:color.frag', command: glslc_spv_cmd, input: 'src/shaders/color.frag', output: '@PLAINNAME@.spv', build_always_stale: true, build_by_default: true)

launch_module = shared_module('launch', name_prefix: '', dependencies: cacao_dep, sources: 'src/Playground.cpp', build_by_default: true)

run_target('bundle_playground', command: ['python', meson.project_source_root()/'playground'/'bundle.py', meson.project_build_root(), backend], depends: [
	launch_module,
	cacao_exe
])

run_target('run_playground', command: ['python', meson.project_source_root()/'playground'/'bundleandrun.py', meson.project_build_root(), backend], depends: [
	launch_module,
	cacao_exe
])

subdir_done()