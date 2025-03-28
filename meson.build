project('cacaoengine', 'cpp', 'c', meson_version: '>=1.2.0', version: '0.1.0.0-a1', license: 'Apache-2.0', default_options: [ 'cpp_std=c++20', 'b_vscrt=from_buildtype' ])

# We are Ninja-only
if get_option('backend') != 'ninja'
	error('Only the Ninja backend is supported. If you\'re trying to use Visual Studio, see the Visual Studio page in the documentation for instructions.')
endif

# Get some options
bt = get_option('buildtype')

# Create extra argument list
dep_compiler_args = []

# Find Python
python = find_program('python3', required: true)

# Resolve component dependencies
build_libs = get_option('build_libs')
build_engine = get_option('build_engine')
build_playground = get_option('build_playground')
build_tools = get_option('build_tools')
if build_engine and not build_libs
	warning('You have selected to build the engine but not supporting libraries. Supporting libraries will be automatically built as they are required.')
	build_libs = true
endif
if build_tools and not build_libs
	warning('You have selected to build tools but not supporting libraries. Supporting libraries will be automatically built as they are required.')
	build_libs = true
endif
if build_playground and not build_engine
	warning('You have selected to build the playground demo but not the engine. The engine will be automatically built as it is required.')
	build_engine = true
endif
if not build_libs and not build_engine and not build_playground and not build_tools
	error('You have not selected anything to build.')
endif

# Generate some variables necessary for CMake to work correctly
cmake_build_type = ''
cmake_msvc_lib = 'MultiThreaded'
if bt.contains('debug')
	if bt == 'debug'
		cmake_build_type = 'Debug'
		cmake_msvc_lib += 'Debug'
	else
		cmake_build_type = 'RelWithDebInfo'
	endif
elif bt == 'plain' or bt == 'minsize'
	cmake_build_type = 'MinSizeRel'
elif bt == 'release'
	cmake_build_type = 'Release'
else
	error('Unsupported build type!')
endif
cmake_msvc_lib += 'DLL'

# Import CMake
cmake = import('cmake')

# Set some Windows-specific options
if host_machine.system() == 'windows'
	add_project_arguments(['-DCACAO_BUILD'], language: ['c', 'cpp'])
	if bt == 'debug'
		add_project_arguments(['-D_ITERATOR_DEBUG_LEVEL=2', '-D_DLL', '-D_MT'], language: ['c', 'cpp'])
	endif
endif

# Set some macOS-specific options
if host_machine.system() == 'darwin'
	add_project_arguments('-fexperimental-library', language: 'cpp')
	add_project_link_arguments('-fexperimental-library', language: 'cpp')
	dep_compiler_args += '-fexperimental-library'
endif

# Define debug macro
if bt.contains('debug')
	add_project_arguments(['-D_DEBUG'], language: 'cpp')
endif

# Configure yaml-cpp
yaml_opts = cmake.subproject_options()
yaml_opts.add_cmake_defines({
	'BUILD_SHARED_LIBS': 'OFF',
	'YAML_CPP_BUILD_TOOLS': 'OFF',
	'YAML_CPP_BUILD_TESTS': 'OFF',
	'CMAKE_POSITION_INDEPENDENT_CODE': 'ON',
	'CMAKE_BUILD_TYPE': cmake_build_type,
	'CMAKE_MSVC_RUNTIME_LIBRARY': cmake_msvc_lib,
	'CMAKE_POLICY_DEFAULT_CMP0091': 'NEW'
})
yaml_opts.set_install(true)
yaml_cpp = cmake.subproject('yaml', options: yaml_opts, required: true).dependency('yaml-cpp')

# Configure Silica
silica = subproject('silica', required: true)
silica_dep = silica.get_variable('silica_dep')

testing = get_option('testing')

# Build components
if build_libs
	subdir('libs')
endif
if build_engine
	subdir('src')
endif
if build_playground
	subdir('playground')
endif
if build_tools
	subdir('tools')
endif