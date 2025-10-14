# Extra args for Windows
cppargs = ['-DCACAO_VER="' + meson.project_version() + '"']
if host_machine.system() == 'windows' and get_option('windows_noconsole') == true
	cppargs += '-DWIN_NOCONSOLE'
endif

# Cacao game runtime
cacao_runtime_exe = executable('cacaort', sources: 'Launcher.cpp', dependencies: [cacao_dep, yaml_cpp, cli11_dep], cpp_args: cppargs, build_rpath: '$ORIGIN', install_rpath: '$ORIGIN', install: true)

# Runtime dependency
cacaort_dep = declare_dependency(sources: ['abi' / 'ABIHandshake.cpp', 'abi' / 'ABI.hpp'], dependencies: cacao_dep, compile_args: bt.contains('debug') ? '-D_DEBUG' : '-DNDEBUG')

# Make list of runtime files
rtfiles_list = [cacao_runtime_exe, libcacao, alsoft_sp.get_variable('openal_lib')]
rtfiles_paths = []
rtfiles_names = []
foreach file : rtfiles_list
	rtfiles_paths += file.full_path()
	if file.full_path() != cacao_runtime_exe.full_path()
		rtfiles_names += fs.name(file.full_path())
	endif
endforeach
rtfiles_paths_str = '\n'.join(rtfiles_paths) + '\n'
rtfiles_data = configuration_data()
rtfiles_data.set('files', rtfiles_paths_str)
rtfiles = configure_file(input: 'rtfiles_template.txt', output: 'rtfiles.txt', configuration: rtfiles_data)