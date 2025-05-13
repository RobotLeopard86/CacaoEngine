# Obtain compilers
cpp = meson.get_compiler('cpp')
objcpp = meson.get_compiler('objcpp')

# Engine headers
engine_headers = include_directories('include')
install_subdir('include' / 'Cacao', install_dir: 'include', install_tag: 'cacao')

# Headers private to modules
module_private_headers = include_directories('modules' / 'private')

# Always-included modules
subdir('modules' / 'shader-tgtgen')
subdir('modules' / 'core')

# Windowing modules
windowing_deps = []
if host_machine.system() == 'windows'
    subdir('modules' / 'win32')
elif host_machine.system() == 'darwin'
    subdir('modules' / 'macos')
elif host_machine.system() == 'linux'
    subdir('modules' / 'linux')
else
    error('Unsupported platform!')
endif

# Graphics backends
backends = get_option('backends')
gfx_deps = []
foreach b : backends
    subdir('modules' / b)
endforeach

# Engine library
libcacao = shared_library('cacao', dependencies: [core_dep, windowing_deps, gfx_deps])
cacao_dep = declare_dependency(include_directories: engine_headers, link_with: libcacao, dependencies: [core_deps])

# Game runtime
subdir('exe')