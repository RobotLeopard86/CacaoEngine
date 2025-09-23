# Extra args for Windows
cppargs = []
if host_machine.system() == 'windows' and get_option('windows_noconsole') == true
	cppargs += '-DWIN_NOCONSOLE'
endif

# Cacao loader library
libloader = static_library('cacaoloader', sources: 'main.cpp', dependencies: cli11_dep, include_directories: engine_headers, cpp_args: cppargs, pic: true, install: true)