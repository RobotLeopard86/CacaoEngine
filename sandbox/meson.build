# Game binary
sandbox = shared_module('sandbox', sources: 'src' / 'sandbox.cpp', dependencies: cacaort_dep, name_prefix: '', install: false)

# Copy runtime executable
custom_target('sandbox_cprt', command: [python, meson.project_source_root() / 'scripts' / 'cprtexe.py', cacao_runtime_exe, meson.project_build_root(), '@OUTPUT@'], output: host_machine.system() == 'windows' ? 'sandbox.exe' : 'sandbox', build_by_default: true)

# Specfile
spec = configuration_data()
spec.set('BINPATH', import('fs').relative_to(sandbox, meson.current_build_dir()))
configure_file(input: 'cacaospec.yml.in', output: 'cacaospec.yml', configuration: spec)