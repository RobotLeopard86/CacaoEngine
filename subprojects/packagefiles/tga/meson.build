project('tga', 'cpp', default_options: ['cpp_std=c++17', 'default_library=static'])

defs = []
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

libtga = library('tga', 'decoder.cpp', 'encoder.cpp', 'image_iterator.cpp', 'stdio.cpp', install: true, cpp_args: defs)

tga_dep = declare_dependency(include_directories: '.', link_with: libtga)