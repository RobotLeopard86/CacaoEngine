project('tga', 'cpp', default_options: ['cpp_std=c++17', 'default_library=static'])

libtga = library('tga', 'decoder.cpp', 'encoder.cpp', 'image_iterator.cpp', 'stdio.cpp', install: true)

tga_dep = declare_dependency(include_directories: '.', link_with: libtga)