# Obtain compilers
cpp = meson.get_compiler('cpp')
objcpp = meson.get_compiler('objcpp')

# Engine headers
engine_headers = include_directories('include', '..' / 'libs' / 'formats' / 'include')
install_subdir('include' / 'Cacao', install_dir: 'include', install_tag: 'cacao')

# Headers private to modules
module_private_headers = include_directories('src' / 'private')

# Always-included modules
subdir('src' / 'shader-tgtgen')
subdir('src' / 'core')

# Windowing modules
windowing_deps = []
if host_machine.system() == 'windows'
    subdir('src' / 'win32')
elif host_machine.system() == 'darwin'
    subdir('src' / 'macos')
elif host_machine.system() == 'linux'
    subdir('src' / 'linux')
else
    error('Unsupported platform!')
endif

# Graphics backends
gfx_deps = []
foreach b : backends
    subdir('src' / b)
endforeach

# Engine library
libcacao = shared_library('cacao', dependencies: [core_dep, windowing_deps, gfx_deps], install: true, install_tag: 'cacao')

# Game loader
subdir('src' / 'loader')

# Final dependency
cacao_dep = declare_dependency(include_directories: engine_headers, link_with: libcacao, link_whole: libloader, dependencies: [core_deps])