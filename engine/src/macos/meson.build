# Frameworks
apple = dependency('appleframeworks', modules : ['Cocoa', 'IOKit', 'CoreFoundation'])

# Keycodes
macos_keycodes_dep = subproject('macos-keycodes', required: true).get_variable('macos_keycodes_dep')

# macOS module
engine_srcs += files('AppleClasses.mm', 'MacOSWindow.mm')
engine_inc += include_directories('.')
engine_deps += [apple, macos_keycodes_dep]