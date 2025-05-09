# Frameworks
apple = dependency('appleframeworks', modules: ['Cocoa', 'IOKit', 'CoreFoundation'])

# Win32 module
libmacos = static_library('cacaomacos', sources: [
	'AppDelegate.mm',
	'MacOSWindow.mm'
], include_directories: [engine_headers, module_private_headers], dependencies: [apple, glm_dep], 
pic: true, install: true, install_tag: 'cacao')

macos_dep = declare_dependency(link_whole: libmacos, dependencies: [apple])
backend_deps += macos_dep