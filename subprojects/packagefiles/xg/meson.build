project('crossguid', 'cpp', version: '0.2.3')

defs = []
deps = []

cpp = meson.get_compiler('cpp')

if host_machine.system() == 'windows'
	defs += '-DGUID_WINDOWS'
	defs += '-D_DLL'
	defs += '-D_MT'
	if get_option('buildtype') == 'debug'
		defs += '-D_DEBUG'
		defs += '-D_ITERATOR_DEBUG_LEVEL=2'
		defs += '-fms-runtime-lib=dll_dbg'
	else
		defs += '-fms-runtime-lib=dll'
	endif
elif host_machine.system() == 'darwin'
	defs += '-DGUID_CFUUID'
	deps += dependency('appleframeworks', modules: ['CoreFoundation'], required: true)
elif host_machine.system() == 'android'
	defs += '-DGUID_ANDROID'
else
	defs += '-DGUID_LIBUUID'
	deps += dependency('libuuid', required: true, static: true)
endif

crossguid = static_library('crossguid', 'src' / 'guid.cpp', include_directories: 'include', cpp_args: defs, dependencies: deps, pic: true)

crossguid_dep = declare_dependency(compile_args: defs, include_directories: 'include', link_with: crossguid)
crossguid_headers_dep = declare_dependency(compile_args: defs, include_directories: 'include')