# Obtain compiler
cpp = meson.get_compiler('cpp')

# Reflection
engine_reflection = custom_target('cacaoengine.astra.[cpphpp]', output: ['cacaoengine.astra.hpp', 'cacaoengine.astra.cpp'],
	input: ['include' / 'Cacao' / 'Actor.hpp'], console: true, command: [astra.get_variable('astra_generator'), '-c', meson.global_build_root(), '-o', 
	meson.current_build_dir(), '-n', 'cacaoengine', '-p', 'Cacao', '-i', '@INPUT@', '--', '-I../libs/asset/include', '-I../libs/image/include', '-I../libs/common/include'], 
	install: true, install_dir: ['include' / 'Cacao', false])

# Engine headers
engine_headers = include_directories('include', '..' / 'libs' / 'asset' / 'include', '..' / 'libs' / 'image' / 'include', '..' / 'libs' / 'common' / 'include')
install_subdir('include' / 'Cacao', install_dir: 'include')
install_subdir(meson.current_build_dir() / 'astra_generated', install_dir: 'include' / 'Cacao')
install_subdir('..' / 'libs' / 'asset' / 'include', install_dir: 'include', strip_directory: true)
install_subdir('..' / 'libs' / 'common' / 'include', install_dir: 'include' / 'libcacaocommon', strip_directory: true)
install_subdir('..' / 'libs' / 'image' / 'include', install_dir: 'include', strip_directory: true)
install_subdir('..' / 'libs' / 'audiodecode' / 'include', install_dir: 'include', strip_directory: true)

# Headers private to modules
module_private_headers = include_directories('src' / 'private')

# Define data for modules to write into
engine_inc = [engine_headers, module_private_headers]
engine_srcs = [engine_reflection]
engine_args = []
engine_args_objcpp = []
engine_deps = []

# Always-included core module
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

# Merge Objective-C++ args
engine_args_objcpp += engine_args
engine_args_objcpp = engine_args_objcpp.flatten()

# Graphics backends
gfx_deps = []
foreach b : backends
    subdir('src' / b)
endforeach

# Engine library
libcacao = shared_library('cacao', sources: engine_srcs, include_directories: engine_inc, dependencies: engine_deps, cpp_args: engine_args, objcpp_args: engine_args_objcpp, install: true)
cacao_dep = declare_dependency(include_directories: engine_headers, link_with: libcacao, dependencies: [crossguid_headers, glm_dep, exathread_dep, commonlib_dep, astra_dep])

# Game runtime
subdir('src' / 'runtime')