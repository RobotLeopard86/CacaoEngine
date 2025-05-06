# Windows subsystem
subsystem = 'console'
if host_machine.system() == 'windows' and get_option('windows_noconsole')
	subsystem = 'windows'
endif

# Final executable
executable(get_option('exe_name'), sources: 'main.cpp', dependencies: [cacao_dep, cli11_dep], include_directories: engine_headers, win_subsystem: subsystem)