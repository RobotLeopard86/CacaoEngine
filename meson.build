project('cacaoengine', 'c', 'cpp', 'objcpp', meson_version: '>=1.9.0', version: '2026.1.0a-a1', license: 'Apache-2.0', default_options: [ 'cpp_std=c++20', 'b_vscrt=from_buildtype', 'default_library=static' ])

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
build_sandbox = get_option('build_sandbox')
build_tools = get_option('build_tools')
if build_engine and not build_libs
	warning('You have selected to build the engine but not supporting libraries. Supporting libraries will be automatically built as they are required.')
	build_libs = true
endif
if build_tools and not build_libs
	warning('You have selected to build tools but not supporting libraries. Supporting libraries will be automatically built as they are required.')
	build_libs = true
endif
if build_sandbox and not build_engine
	warning('You have selected to build the sandbox demo but not the engine. The engine will be automatically built as it is required.')
	build_engine = true
endif
if not build_libs and not build_engine and not build_sandbox and not build_tools
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

# Import CMake and filesystem modules
cmake = import('cmake')
fs = import('fs')

# Set some Windows-specific options
if host_machine.system() == 'windows'
	add_project_arguments(['-DCACAO_BUILD', '-D_DLL', '-D_MT', '-DHAS_WIN32', '-D_CRT_SECURE_NO_WARNINGS'], language: ['c', 'cpp'])
	if bt == 'debug'
		add_project_arguments(['-D_ITERATOR_DEBUG_LEVEL=2', '-fms-runtime-lib=dll_dbg'], language: ['c', 'cpp'])
	else
		add_project_arguments('-fms-runtime-lib=dll', language: ['c', 'cpp'])
	endif
endif

# Set some macOS-specific options
if host_machine.system() == 'darwin'
	add_project_arguments(['-fexperimental-library', '-DHAS_MACOS'], language: 'cpp')
	add_project_link_arguments('-fexperimental-library', language: 'cpp')
	dep_compiler_args += '-fexperimental-library'
endif

# Set some Linux-specific options
if host_machine.system() == 'linux'
	linux_windowing = get_option('linux_windowing')
	add_project_arguments('-DHAS_LINUX', language: 'cpp')
	foreach lw : linux_windowing
		add_project_arguments('-DHAS_' + lw.to_upper(), language: ['c', 'cpp'])
	endforeach
endif

# Get graphics backends
backends = get_option('backends')
if build_engine and backends.length() <= 0
    error('At least one graphics backend must be selected!')
endif
backend_keymap = {
	'opengl': 'GL',
	'vulkan': 'VK'
}
foreach b : backends
	add_project_arguments('-DHAS_' + backend_keymap[b], language: ['c', 'cpp'])
endforeach

# Define debug macro
if bt.contains('debug')
	add_project_arguments(['-D_DEBUG'], language: 'cpp')
endif

# Slang compiler and libraries
slang = subproject('slang', required: true)
slang_dep = slang.get_variable('slang_dep')

# Process shader base module file
shaderbasemod = custom_target('cacaoshaderbase', input: meson.project_source_root() / 'tools' / 'shaderc' / 'cacaoshaderbase.slang', output: 'cacaoshaderbase.inc', command: [python, meson.project_source_root() / 'scripts' / 'includable.py', '@INPUT@', '@OUTDIR@'])

# Configure yaml-cpp
yaml_cpp = subproject('yaml-cpp', required: true).get_variable('yaml_dep')

# Configure Silica
silica = subproject('silica', required: true, default_options: {
	'example': false,
	'generator': true
})
silica_dep = silica.get_variable('silica_dep')

# Configure crossguid
crossguid_sp = subproject('crossguid', required: true)
crossguid = crossguid_sp.get_variable('crossguid_dep')
xg_headers_dep = crossguid_sp.get_variable('crossguid_headers_dep')

# Configure SPIRV-Cross
spv_cross_sp = subproject('spirv-cross', required: true)
spv_core = spv_cross_sp.get_variable('spv_core')
spv_cpp = spv_cross_sp.get_variable('spv_cpp')
spv_rfl = spv_cross_sp.get_variable('spv_reflect')
spv_util = spv_cross_sp.get_variable('spv_util')
spv_glsl = spv_cross_sp.get_variable('spv_glsl')
spv_hlsl = spv_cross_sp.get_variable('spv_hlsl')
spv_msl = spv_cross_sp.get_variable('spv_msl')
spirv_cross = declare_dependency(dependencies: [spv_core, spv_cpp, spv_rfl, spv_util, spv_glsl, spv_hlsl, spv_msl])

# CLI11
cli11_dep = subproject('cli11', required: true).get_variable('CLI11_dep')

testing = get_option('testing')

# Build components
if build_libs
	subdir('libs')
endif
if build_tools
	subdir('tools')
endif
if build_engine
	subdir('engine')
endif
if build_sandbox
	subdir('sandbox')
endif