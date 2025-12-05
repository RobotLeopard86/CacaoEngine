# Game binary
sandbox = shared_module('sandbox', sources: 'src' / 'sandbox.cpp', dependencies: cacaort_dep, name_prefix: '', install: false)

# Copy runtime executable
custom_target('sandbox_rtsetup', command: [python, rtdata['setup_script'], rtdata['filelist'], meson.current_build_dir(), '@OUTPUT0@'], output: [host_machine.system() == 'windows' ? 'sandbox.exe' : 'sandbox', rtdata['filenames']], build_by_default: true, depends: rtdata['files'])

# Specfile
spec = configuration_data()
spec.set('BINPATH', fs.relative_to(sandbox, meson.current_build_dir()))
spec.set('ID', 'net.rl86.CacaoSandbox')
spec.set('TITLE', 'Cacao Engine Sandbox Application')
spec.set('VERSION', meson.project_version())
configure_file(input: 'cacaospec.yml.in', output: 'cacaospec.yml', configuration: spec)