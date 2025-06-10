# Wayland and libdecor (decoration manager)
wayland_client  = dependency('wayland-client', required: true)
xkbcommon_dep   = dependency('xkbcommon', required: true)

# Configure libdecor
libdecor = subproject('libdecor', required: true, default_options: {
	'demo': false,
	'gtk': 'enabled'
})
libdecor_dep = libdecor.get_variable('libdecor_dep')
desktopsettings = libdecor.get_variable('desktop_settings_dep')

# xdg-outputprotocol
wl = import('unstable-wayland')
xdgoutxml = wl.find_protocol('xdg-output', state: 'unstable', version: 1)
xdg_output = wl.scan_xml(xdgoutxml, client: true, include_core_only: false)
xdg_output_h = xdg_output[1]
xdg_output_dep = declare_dependency(sources: xdg_output_h)

# Wayland module
libwl = static_library('cacaowayland', sources: [
	'WlWindow.cpp', xdg_output
], include_directories: [engine_headers, module_private_headers], dependencies: [wayland_client, xkbcommon_dep, libdecor_dep, desktopsettings, glm_dep, xg_headers_dep], 
pic: true)

wl_dep = declare_dependency(link_whole: libwl, dependencies: [wayland_client, xkbcommon_dep, libdecor_dep, desktopsettings, glm_dep, xg_headers_dep])
linux_deps += wl_dep
linux_deps += xdg_output_dep