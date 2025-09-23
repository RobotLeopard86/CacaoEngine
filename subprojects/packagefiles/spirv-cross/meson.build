project('spirv-cross', 'cpp', license: 'MIT', default_options: [ 'cpp_std=c++17', 'default_library=static', 'warning_level=2' ])

# Windows args
defs = ['-Wshadow', '-Wno-deprecated-declarations', '-Wshorten-64-to-32']
if host_machine.system() == 'windows'
	defs += '-D_DLL'
	defs += '-D_MT'
	if get_option('buildtype') == 'debug'
		defs += '-D_DEBUG'
		defs += '-D_ITERATOR_DEBUG_LEVEL=2'
		defs += '-fms-runtime-lib=dll_dbg'
	else
		defs += '-fms-runtime-lib=dll'
	endif
endif

# core
libcore = library('spirv_cross_core', sources: [
	'GLSL.std.450.h',
	'spirv_common.hpp',
	'spirv_cross_containers.hpp',
	'spirv_cross_error_handling.hpp',
	'spirv.hpp',
	'spirv_cross.hpp',
	'spirv_cross.cpp',
	'spirv_parser.hpp',
	'spirv_parser.cpp',
	'spirv_cross_parsed_ir.hpp',
	'spirv_cross_parsed_ir.cpp',
	'spirv_cfg.hpp',
	'spirv_cfg.cpp'
], pic: true, cpp_args: defs)
spv_core = declare_dependency(link_with: libcore, include_directories: '.')

# cpp
libcpp = library('spirv_cross_cpp', sources: [
	'spirv_cpp.cpp',
	'spirv_cpp.hpp'
], pic: true, cpp_args: defs)
spv_cpp = declare_dependency(link_with: libcpp, include_directories: '.')

# glsl
libglsl = library('spirv_cross_glsl', sources: [
	'spirv_glsl.cpp',
	'spirv_glsl.hpp'
], pic: true, cpp_args: defs)
spv_glsl = declare_dependency(link_with: libglsl, include_directories: '.')

# hlsl
libhlsl = library('spirv_cross_hlsl', sources: [
	'spirv_hlsl.cpp',
	'spirv_hlsl.hpp'
], pic: true, cpp_args: defs)
spv_hlsl = declare_dependency(link_with: libhlsl, include_directories: '.')

# msl
libmsl = library('spirv_cross_msl', sources: [
	'spirv_msl.cpp',
	'spirv_msl.hpp'
], pic: true, cpp_args: defs)
spv_msl = declare_dependency(link_with: libmsl, include_directories: '.')

# reflect
libreflect = library('spirv_cross_reflect', sources: [
	'spirv_reflect.cpp',
	'spirv_reflect.hpp'
], pic: true, cpp_args: defs)
spv_reflect = declare_dependency(link_with: libreflect, include_directories: '.')

# util
libutil = library('spirv_cross_util', sources: [
	'spirv_cross_util.cpp',
	'spirv_cross_util.hpp'
], pic: true, cpp_args: defs)
spv_util = declare_dependency(link_with: libutil, include_directories: '.')