# Wayland and libdecor (decoration manager)
wayland_client  = dependency('wayland-client', required: true)
xkbcommon_dep   = dependency('xkbcommon', required: true)

# Configure libdecor
libdecor = subproject('libdecor', required: true, default_options: {
	'demo': false,
	'gtk': 'disabled'
})
libdecor_dep = libdecor.get_variable('libdecor_dep')
desktopsettings = libdecor.get_variable('desktop_settings_dep')

# xdg-output protocol
wl = import('unstable-wayland')
xdgoutxml = wl.find_protocol('xdg-output', state: 'unstable', version: 1)
xdg_output = wl.scan_xml(xdgoutxml, client: true, include_core_only: false)
xdg_output_h = xdg_output[1]
xdg_output_dep = declare_dependency(sources: xdg_output_h)

# Wayland module
engine_srcs += files('WlWindow.cpp')
engine_srcs += xdg_output
engine_inc += include_directories('.')
engine_deps += [wayland_client, xkbcommon_dep, libdecor_dep, desktopsettings, xdg_output_dep]