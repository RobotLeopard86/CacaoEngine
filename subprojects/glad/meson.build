project('glad', 'c')

what = get_option('what')
sources = []

if what == 'gl'
	sources = [ 'src/gl.c' ]
endif

glad = static_library('glad_' + what, sources: sources, include_directories: 'include', pic: true)

glad_dep = declare_dependency(include_directories: 'include', link_with: glad)