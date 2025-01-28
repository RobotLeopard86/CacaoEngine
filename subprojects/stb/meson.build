project('stb', 'c')

stb = static_library('stb', 'stb.c', pic: true)

stb_dep = declare_dependency(include_directories: '.', link_with: stb)