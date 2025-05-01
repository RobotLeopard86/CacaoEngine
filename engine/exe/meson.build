# Windows subsystem
subsystem = 'console'
if host_machine.system() == 'windows' and get_option('windows_noconsole')
	subsystem = 'windows'
endif

# Final executable
executable(get_option('exe_name'), sources: [], dependencies: [core_dep, cli11_dep])