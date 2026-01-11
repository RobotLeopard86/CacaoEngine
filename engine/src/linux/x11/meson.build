# xcb
xcb_dep         = dependency('xcb', required: true)
xcb_ks_dep         = dependency('xcb-keysyms', required: true)
xcb_atom_dep         = dependency('xcb-atom', required: true)
xcb_randr_dep         = dependency('xcb-randr', required: true)

# X11 module
engine_srcs += files('XWindow.cpp')
engine_inc += include_directories('.')
engine_deps += [xcb_dep, xcb_ks_dep, xcb_atom_dep, xcb_randr_dep]
