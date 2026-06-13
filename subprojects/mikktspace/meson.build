project('mikktspace', 'c')

mikktspace_lib = static_library('mikktspace', sources: 'mikktspace.c', include_directories: '.', pic: true)
mikktspace_dep = declare_dependency(include_directories: '.', link_with: mikktspace_lib)