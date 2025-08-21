project('glad-wgl', 'c')

gladwgl = static_library('gladwgl', sources: 'src/wgl.c', include_directories: 'include', pic: true)
glad_wgl_dep = declare_dependency(include_directories: 'include', link_with: gladwgl)