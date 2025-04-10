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
	'SLANG_SLANG_LLVM_FLAVOR': 'DISABLE',
	'CMAKE_POSITION_INDEPENDENT_CODE': 'ON',
	'CMAKE_BUILD_TYPE': cmake_build_type,
	'CMAKE_MSVC_RUNTIME_LIBRARY': cmake_msvc_lib,
	'CMAKE_POLICY_DEFAULT_CMP0091': 'NEW'
})
if host_machine.system() == 'darwin'
	slang_opts.append_compile_args('cpp', '-Wno-#warnings')
endif
slang_sp = cmake.subproject('slang', options: slang_opts, required: true)
slang_core = slang_sp.dependency('core')
slang_compiler_core = slang_sp.dependency('compiler_core')
slang_lz4 = slang_sp.dependency('lz4_static')
slang_miniz = slang_sp.dependency('miniz')
slang_prelude = slang_sp.dependency('prelude')
slang_rt = slang_sp.dependency('slang_rt')
slang_embedded_core_module_source = slang_sp.dependency('slang_embedded_core_module_source')
slang_embedded_core_module = slang_sp.dependency('slang_embedded_core_module')
slang_common_objects = slang_sp.dependency('slang_common+objects')
slang = slang_sp.dependency('slang')

# Shader compiler executable
shaderc = executable('ce-shaderc', sources: [
	'main.cpp'
], dependencies: [
	cli11_dep, spinners_dep, slang_core, slang_compiler_core, slang_lz4, slang_miniz, slang_prelude, slang_rt, slang_embedded_core_module_source, slang_embedded_core_module, slang_common_objects, slang
], cpp_args: ['-DCACAO_VER="' + meson.project_version() + '"', '-DCOMPILER_VER="1.0.0"'])