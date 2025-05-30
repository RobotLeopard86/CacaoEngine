# Subsystems
linux_deps = []
if linux_windowing.length() < 1
	error('At least one Linux windowing system must be selected!')
endif
foreach lw : linux_windowing
	subdir(lw)
endforeach

# Linux common library
liblinux = static_library('cacaolinux', sources: 'LinuxRouter.cpp', include_directories:
	[engine_headers, module_private_headers], dependencies: [linux_deps, glm_dep, xg_headers_dep], 
	pic: true, install: true, install_tag: 'cacao')

linux_dep = declare_dependency(link_whole: liblinux)
windowing_deps += linux_dep