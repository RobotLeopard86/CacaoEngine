project('opusfile', 'c', version: 'main', license: 'bsd-like')

ogg_dep = dependency('ogg')
opus_dep = dependency('opus')

cc = meson.get_compiler('c')
m_dep = cc.find_library('m', required: false)
lrintf = cc.has_function('lrintf', dependencies: m_dep)

if lrintf
	add_project_arguments('-DOP_HAVE_LRINTF', language: 'c')
endif

opusfile = library('opusfile', pic: true, sources: [
	'include' / 'opusfile.h',
	'src' / 'info.c',
	'src' / 'internal.c',
	'src' / 'internal.h',
	'src' / 'opusfile.c',
	'src' / 'stream.c'
], dependencies: [ogg_dep, opus_dep, m_dep], gnu_symbol_visibility: 'hidden', soversion: 1, include_directories: 'include')

opusfile_dep = declare_dependency(link_with: opusfile, include_directories: 'include')