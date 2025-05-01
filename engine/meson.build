# Obtain compilers
cpp = meson.get_compiler('cpp')
objcpp = meson.get_compiler('objcpp')

# Engine headers
engine_headers = include_directories('include')

# Always-included modules
subdir('modules' / 'shader-tgtgen')
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

# Game runtime
#subdir('modules' / 'runtime')