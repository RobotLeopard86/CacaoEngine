project('cmark', 'c', license: 'BSD-2-Clause', version: '0.29.0.gfm.13', default_options: ['c_std=c99', 'warning_level=3'])

incdirs = include_directories('.', 'src' / 'include', 'extensions/include')

cc = meson.get_compiler('c')
add_project_arguments(['-D_CRT_SECURE_NO_WARNINGS', '-DCMARK_USE_CMAKE_HEADERS', '-DCMARK_GFM_STATIC_DEFINE'], language: 'c')
if cc.get_id() == 'msvc'
	add_project_arguments(['/wd5105', '/wd4706', '/wd4221', '/wd4204', '/wd4100'], language: 'c')
else
	add_project_arguments(['-pedantic', '-Wno-unused-parameter'], language: 'c')
endif

if get_option('buildtype') == 'debug'
	add_project_arguments(['-DCMARK_DEBUG_NODES', '-DDEBUG'], language: 'c')
endif

config_data = configuration_data()
config_data.set('CMARK_THREADING', 'false')
config_h = configure_file(input: 'src' / 'config.h.in', output: 'config.h', format: 'cmake', configuration: config_data)

cmark_gfm = static_library('cmark-gfm', sources: [
	'src' / 'arena.c',
	'src' / 'blocks.c',
	'src' / 'buffer.c',
	'src' / 'cmark.c',
	'src' / 'cmark_ctype.c',
	'src' / 'commonmark.c',
	'src' / 'footnotes.c',
	'src' / 'houdini_href_e.c',
	'src' / 'houdini_html_e.c',
	'src' / 'houdini_html_u.c',
	'src' / 'html.c',
	'src' / 'inlines.c',
	'src' / 'iterator.c',
	'src' / 'latex.c',
	'src' / 'linked_list.c',
	'src' / 'man.c',
	'src' / 'map.c',
	'src' / 'node.c',
	'src' / 'plaintext.c',
	'src' / 'plugin.c',
	'src' / 'references.c',
	'src' / 'registry.c',
	'src' / 'render.c',
	'src' / 'scanners.c',
	'src' / 'syntax_extension.c',
	'src' / 'utf8.c',
	'src' / 'xml.c'
], include_directories: incdirs, pic: true)

cmark_extensions = static_library('cmark-gfm-extensions', sources: [
	'extensions' / 'autolink.c',
	'extensions' / 'core-extensions.c',
	'extensions' / 'ext_scanners.c',
	'extensions' / 'ext_scanners.h',
	'extensions' / 'strikethrough.c',
	'extensions' / 'table.c',
	'extensions' / 'tagfilter.c',
	'extensions' / 'tasklist.c',
], include_directories: incdirs, pic: true, link_with: cmark_gfm)

cmark_dep = declare_dependency(include_directories: incdirs, compile_args: ['-D_CRT_SECURE_NO_WARNINGS', '-DCMARK_USE_CMAKE_HEADERS', '-DCMARK_GFM_STATIC_DEFINE'], link_with: [cmark_gfm, cmark_extensions])