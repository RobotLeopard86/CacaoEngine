# kernel32.lib and user32.lib
kernel32 = cpp.find_library('kernel32', required: true)
user32 = cpp.find_library('user32', required: true)

# Win32 module
engine_srcs += files('Win32Window.cpp')
engine_inc += include_directories('.')
engine_deps += [kernel32, user32]