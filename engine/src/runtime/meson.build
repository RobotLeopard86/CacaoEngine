# Extra args for Windows
cppargs = ['-DCACAO_VER="' + meson.project_version() + '"']
if host_machine.system() == 'windows' and get_option('windows_noconsole') == true
	cppargs += '-DWIN_NOCONSOLE'
endif

# Cacao game runtime
cacao_runtime_exe = executable('cacaort', sources: 'Launcher.cpp', dependencies: [cacao_dep, yaml_cpp, cli11_dep], cpp_args: cppargs, install: true)

# Runtime dependency
cacaort_dep = declare_dependency(sources: ['abi' / 'ABIHandshake.cpp', 'abi' / 'ABI.hpp'], dependencies: cacao_dep, compile_args: bt.contains('debug') ? '-D_DEBUG' : '-DNDEBUG')