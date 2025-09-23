# Frameworks
apple = dependency('appleframeworks', modules: ['Cocoa', 'IOKit', 'CoreFoundation'])

# macOS module
engine_srcs += files('AppDelegate.mm', 'MacOSWindow.mm')
engine_inc += include_directories('.')
engine_deps += [apple]