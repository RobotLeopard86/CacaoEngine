# Game binary
sandbox = shared_module('sandbox', sources: 'src' / 'sandbox.cpp', dependencies: cacaort_dep, name_prefix: '', install: false)

# Copy runtime executable
custom_target('sandbox_rtsetup', command: [python, meson.project_source_root() / 'scripts' / 'rtsetup.py', rtfiles, meson.current_build_dir(), '@OUTPUT0@'], output: [host_machine.system() == 'windows' ? 'sandbox.exe' : 'sandbox', rtfiles_names], build_by_default: true, depends: rtfiles_list)

# Specfile
spec = configuration_data()
spec.set('BINPATH', fs.relative_to(sandbox, meson.current_build_dir()))
configure_file(input: 'cacaospec.yml.in', output: 'cacaospec.yml', configuration: spec)