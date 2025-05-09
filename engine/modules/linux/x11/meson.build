# xcb
xcb_dep         = dependency('xcb', required: true, static: true)
xcb_ks_dep         = dependency('xcb-keysyms', required: true, static: true)
xcb_atom_dep         = dependency('xcb-atom', required: true, static: true)

# X11 module
libx11 = static_library('cacaox11', sources: [
	'XWindow.cpp'
], include_directories: [engine_headers, module_private_headers], dependencies: [xcb_dep, xcb_ks_dep, glm_dep], 
pic: true, install: true, install_tag: 'cacao')

x11_dep = declare_dependency(link_whole: libx11)
linux_deps += x11_dep