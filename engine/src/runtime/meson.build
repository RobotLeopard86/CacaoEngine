# Extra args for Windows
cppargs = ['-DCACAO_VER="' + meson.project_version() + '"']
if host_machine.system() == 'windows' and get_option('windows_noconsole') == true
	cppargs += '-DWIN_NOCONSOLE'
endif

# Dynalo (dynamic library loader) dependency
dynalo_dep = subproject('dynalo', required: true).get_variable('dynalo_dep')

# Cacao game runtime
rpath = host_machine.system() == 'darwin' ? '@executable_path' : '$ORIGIN'
cacao_runtime_exe = executable('cacaort', sources: [
	'Launcher.cpp',
	'Setup.cpp'
], dependencies: [cacao_dep, yaml_cpp, cli11_dep, dynalo_dep], cpp_args: cppargs, build_rpath: rpath, install_rpath: rpath, install: true)

# Runtime dependency
cacaort_dep = declare_dependency(sources: ['abi' / 'ABIHandshake.cpp', 'abi' / 'ABI.hpp'], dependencies: cacao_dep, compile_args: bt.contains('debug') ? '-D_DEBUG' : '-DNDEBUG')

# License file
licenses = custom_target('cacaolicenses', command: [python, meson.project_source_root() / 'scripts' / 'licensecombiner.py', '@OUTPUT@'], output: 'cacaolicenses.txt', build_by_default: true, input: [
	meson.project_source_root() / 'scripts' / 'licensecombiner.py',
	meson.project_source_root() / 'licenses' / 'CANARY.txt'
])

# Make list of runtime files
rtfiles_list = [cacao_runtime_exe, libcacao, alsoft_sp.get_variable('openal_lib'), licenses]
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

# Runtime stuff
meson.override_find_program('_cacao_rtsetup', files(meson.project_source_root() / 'scripts' / 'rtsetup.py'))
rtdata = {
	'setup_script': find_program('_cacao_rtsetup', required: true),
	'filelist': rtfiles,
	'files': rtfiles_list,
	'filenames': rtfiles_names
}