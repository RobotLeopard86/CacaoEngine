# Game binary
sandbox = shared_module('sandbox', sources: 'src' / 'sandbox.cpp', dependencies: cacao_dep, name_prefix: '', install: false)

# Specfile
spec = configuration_data()
spec.set('BINPATH', import('fs').relative_to(sandbox, meson.current_build_dir()))
configure_file(input: 'cacaospec.yml.in', output: 'cacaospec.yml', configuration: spec)