# GLAD
glad_dep = subproject('glad').get_variable('glad_dep')

# Sources
gl_sources = [
	'OpenGLModule.cpp',
	#'wsi' / 'Common.cpp'
]
gl_inc = [engine_headers, module_private_headers]
gl_deps = [glad_dep, glm_dep, xg_headers_dep]

# Graphics connection
if host_machine.system() == 'windows'
    gl_sources += 'wsi' / 'Win32WSI.cpp'
	gl_inc += '..' / 'win32'
elif host_machine.system() == 'darwin'
    #gl_sources += 'wsi' / 'MacOSWSI.cpp'
	gl_inc += '..' / 'macos'
elif host_machine.system() == 'linux'
	gl_inc += '..' / 'linux'
    if linux_windowing.contains('x11')
		gl_sources +=  'wsi' / 'XWSI.cpp'
		gl_dep += [xcb_dep, xcb_ks_dep, xcb_atom_dep, xcb_randr_dep]
	endif
	if linux_windowing.contains('wayland')
		gl_sources += 'wsi' / 'WaylandWSI.cpp'
		gl_dep += [wayland_client, xdg_output_dep, xkbcommon_dep, libdecor_dep]
	endif
	gl_sources += 'wsi' / 'LinuxWSI.cpp'
else
    error('Unsupported platform for OpenGL module!')
endif

# OpenGL module
glmodule = shared_library('cacaogl', sources: gl_sources, include_directories: gl_inc, 
	dependencies: gl_deps, install: true, install_tag: 'cacao', override_options: ['b_lundef=false'])
gl_dep = declare_dependency(link_with: glmodule)
gfx_deps += gl_dep