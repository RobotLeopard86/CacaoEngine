project('glad', 'c')

glad = static_library('glad', sources: 'src/gl.c', include_directories: 'include', pic: true)
glad_dep = declare_dependency(include_directories: 'include', link_with: glad)