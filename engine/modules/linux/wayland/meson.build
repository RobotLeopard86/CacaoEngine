# Wayland and libdecor (decoration manager)
wayland_client  = dependency('wayland-client', required: true, static: true)
xkbcommon_dep   = dependency('xkbcommon', required: true, static: true)

# Configure libdecor
libdecor = subproject('libdecor', required: true, default_options: {
	'demo': false,
	'gtk': 'disabled'
})
libdecor_dep = libdecor.get_variable('libdecor_dep')
desktopsettings = libdecor.get_variable('desktop_settings_dep')

# Wayland module
libwl = static_library('cacaowayland', sources: [
	'WlWindow.cpp'
], include_directories: [engine_headers, module_private_headers], dependencies: [wayland_client, xkbcommon_dep, libdecor_dep, desktopsettings, glm_dep], 
pic: true, install: true, install_tag: 'cacao')

wl_dep = declare_dependency(link_whole: libwl, dependencies: [wayland_client, xkbcommon_dep, libdecor_dep, desktopsettings, glm_dep])
linux_deps += wl_dep