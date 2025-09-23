project('glad-gl', 'c')

gladgl = static_library('gladgl', sources: 'src/gl.c', include_directories: 'include', pic: true)
glad_gl_dep = declare_dependency(include_directories: 'include', link_with: gladgl)