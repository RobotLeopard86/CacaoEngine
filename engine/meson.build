# Obtain compilers
cpp = meson.get_compiler('cpp')
objcpp = meson.get_compiler('objcpp')

# Engine headers
engine_headers = include_directories('include')

# Always-included core
subdir('modules' / 'core')

# Windowing modules
if host_machine.system() == 'windows'
    subdir('modules' / 'win32')
elif host_machine.system() == 'darwin'
    subdir('modules' / 'macos')
elif host_machine.system() == 'linux'
    linwin = get_option('linux_windowing')
    foreach w : linwin
        subdir('modules' / w)
    endforeach
else
    error('Unsupported platform!')
endif

# Graphics backends
backends = get_option('backends')
foreach b : backends
    subdir('modules' / b)
endforeach

# Windows subsystem
subsystem = 'console'
if host_machine.system() == 'windows' and get_option('windows_noconsole')
	subsystem = 'windows'
endif

# Final executable
executable(get_option('exe_name'), 'Startup.cpp', dependencies: [core_dep])