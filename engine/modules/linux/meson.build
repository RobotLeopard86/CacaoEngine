# Subsystems
linux_windowing = get_option('linux_windowing')
linux_deps = []
defs= []
if linux_windowing.length() < 1
	error('At least one Linux windowing system must be selected!')
endif
foreach lw : linux_windowing
	subdir(lw)
	defs += '-DHAS_' + lw.to_upper()
endforeach

# Linux common library
liblinux = static_library('cacaolinux', sources: 'LinuxRouter.cpp', include_directories: [engine_headers, module_private_headers], dependencies: [linux_deps, glm_dep], 
pic: true, install: true, install_tag: 'cacao', cpp_args: defs)

linux_dep = declare_dependency(link_whole: liblinux)
backend_deps += linux_dep