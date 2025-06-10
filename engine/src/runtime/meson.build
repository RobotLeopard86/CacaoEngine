# Windows subsystem
subsystem = 'console'
if host_machine.system() == 'windows' and get_option('windows_noconsole')
	subsystem = 'windows'
endif
lnk_args = subsystem == 'windows' ? ['--entry', 'mainCRTStartup'] : []

# Final executable
cacao_exe = executable(get_option('exe_name'), sources: 'main.cpp', 
	dependencies: [cacao_dep, cli11_dep], include_directories: engine_headers, 
	win_subsystem: subsystem, export_dynamic: true, link_args: lnk_args, install: true, install_tag: 'cacao')