project('cacaoengine', 'c', 'cpp', 'objcpp', meson_version: '>=1.2.0', version: '2025.1.0a-a1', license: 'Apache-2.0', default_options: [ 'cpp_std=c++20', 'b_vscrt=from_buildtype', 'default_library=static' ])

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

# Set some Windows-specific options
if host_machine.system() == 'windows'
	add_project_arguments(['-DCACAO_BUILD', '-D_DLL', '-D_MT', '-DHAS_WIN32'], language: ['c', 'cpp'])
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
if backends.length() == 0
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

# Slang compiler
slang_opts = cmake.subproject_options()
slang_opts.add_cmake_defines({
	'SLANG_ENABLE_DXIL': 'FALSE',
	'SLANG_ENABLE_GFX': 'FALSE',
	'SLANG_ENABLE_SLANGC': 'FALSE',
	'SLANG_ENABLE_SLANGD': 'FALSE',
	'SLANG_ENABLE_SLANG_GLSLANG': 'FALSE',
	'SLANG_ENABLE_TESTS': 'FALSE',
	'SLANG_ENABLE_EXAMPLES': 'OFF',
	'SLANG_LIB_TYPE': 'STATIC',
	'SLANG_ENABLE_RELEASE_DEBUG_INFO': 'FALSE',
	'SLANG_ENABLE_CUDA': 'FALSE',
	'SLANG_ENABLE_OPTIX': 'FALSE',
	'SLANG_ENABLE_NVAPI': 'FALSE',
	'SLANG_ENABLE_AFTERMATH': 'FALSE',
	'SLANG_ENABLE_XLIB': 'FALSE',
	'SLANG_ENABLE_SLANG_RHI': 'FALSE',
	'SLANG_EXCLUDE_DAWN': 'TRUE',
	'SLANG_EXCLUDE_TINT': 'TRUE',
	'SLANG_SLANG_LLVM_FLAVOR': 'DISABLE',
	'CMAKE_POSITION_INDEPENDENT_CODE': 'ON',
	'CMAKE_BUILD_TYPE': cmake_build_type,
	'CMAKE_MSVC_RUNTIME_LIBRARY': cmake_msvc_lib,
	'CMAKE_POLICY_DEFAULT_CMP0091': 'NEW'
})
if host_machine.system() == 'darwin'
	slang_opts.append_compile_args('cpp', '-Wno-#warnings')
endif
slang_opts.set_install(false)
slang_sp = cmake.subproject('slang', options: slang_opts, required: true)
slang_core = slang_sp.dependency('core')
slang_compiler_core = slang_sp.dependency('compiler_core')
slang_lz4 = slang_sp.dependency('lz4_static')
slang_miniz = slang_sp.dependency('miniz')
slang_prelude = slang_sp.dependency('prelude')
slang_rt = slang_sp.dependency('slang_rt')
slang_embedded_core_module_source = slang_sp.dependency('slang_embedded_core_module_source')
slang_embedded_core_module = slang_sp.dependency('slang_embedded_core_module')
slang_common_objects = slang_sp.dependency('slang_common_objects')
slang_capability_lookup = slang_sp.dependency('slang_capability_lookup')
slang_lookup_tables = slang_sp.dependency('slang_lookup_tables')
slang = slang_sp.dependency('slang')
slang_dep = declare_dependency(dependencies: [slang_core, slang_compiler_core, slang_lz4, slang_miniz, slang_prelude, slang_rt, 
	slang_embedded_core_module_source, slang_embedded_core_module, slang_common_objects, slang_capability_lookup, slang_lookup_tables, slang])

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