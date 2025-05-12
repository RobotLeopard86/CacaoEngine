# kernel32.lib and user32.lib
kernel32 = cpp.find_library('kernel32', required: true)
user32 = cpp.find_library('user32', required: true)

# Win32 module
libwin32 = static_library('cacaowin32', sources: [
	'Win32Window.cpp'
], include_directories: [engine_headers, module_private_headers], dependencies: [kernel32, user32, glm_dep], 
pic: true, install: true, install_tag: 'cacao')

win32_dep = declare_dependency(link_whole: libwin32)
windowing_deps += win32_dep