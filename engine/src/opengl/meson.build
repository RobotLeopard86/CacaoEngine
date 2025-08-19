# GLAD
glad_dep = subproject('glad').get_variable('glad_dep')

# eternal (for constexpr map)
eternal_dep = subproject('eternal').get_variable('eternal_dep')

# Sources
gl_sources = [
	'OpenGLModule.cpp',
	'GPU.cpp',
	'impls' / 'OpenGLMesh.cpp',
	'impls' / 'OpenGLTex2D.cpp',
	'impls' / 'OpenGLCubemap.cpp'
]
gl_inc = [engine_headers, module_private_headers]
gl_deps = [glad_dep, glm_dep, eternal_dep, xg_headers_dep]

# Graphics connection
if host_machine.system() == 'windows'
    #gl_sources += 'wsi' / 'Win32WSI.cpp'
	gl_inc += '..' / 'win32'
elif host_machine.system() == 'darwin'
    gl_sources += 'ctx' / 'MacOS.mm'
	gl_inc += '..' / 'macos'
elif host_machine.system() == 'linux'
	gl_inc += '..' / 'linux'
	gl_deps += dependency('egl', required: true)
    if linux_windowing.contains('x11')
		gl_deps += [xcb_dep, xcb_ks_dep, xcb_atom_dep, xcb_randr_dep]
	endif
	if linux_windowing.contains('wayland')
		gl_deps += [wayland_client, xdg_output_dep, xkbcommon_dep, libdecor_dep, dependency('wayland-egl', required: true)]
	endif
	gl_sources += 'ctx' / 'EGL.cpp'
else
    error('Unsupported platform for OpenGL module!')
endif

# OpenGL module
glmodule = static_library('cacaogl', sources: gl_sources, include_directories: gl_inc, 
	dependencies: gl_deps, install: true, install_tag: 'cacao', pic: true, objcpp_args: ['-Wno-deprecated-declarations', '-DGL_SILENCE_DEPRECATION'])
gl_dep = declare_dependency(link_whole: glmodule)
gfx_deps += gl_dep