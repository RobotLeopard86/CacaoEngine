# Obtain compilers
cpp = meson.get_compiler('cpp')
objcpp = meson.get_compiler('objcpp')

# Engine headers
engine_headers = include_directories('include', '..' / 'libs' / 'formats' / 'include', '..' / 'libs' / 'image' / 'include')
install_subdir('include' / 'Cacao', install_dir: 'include')

# Headers private to modules
module_private_headers = include_directories('src' / 'private')

# Define data for modules to write into
engine_inc = [engine_headers, module_private_headers]
engine_srcs = []
engine_args = []
engine_args_objcpp = []
engine_deps = []

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
libcacao = shared_library('cacao', sources: engine_srcs, include_directories: engine_inc, dependencies: engine_deps, cpp_args: engine_args, objcpp_args: engine_args_objcpp, install: true)

# Game loader
subdir('src' / 'loader')

# Loader headers
loader_headers = include_directories('src' / 'loader' / 'include')
install_subdir('src' / 'loader' / 'include', strip_directory: true, install_dir: 'include')

# Final dependency
cacao_dep = declare_dependency(include_directories: [engine_headers, loader_headers], link_with: libcacao, link_whole: libloader, dependencies: [xg_headers_dep, glm_dep])