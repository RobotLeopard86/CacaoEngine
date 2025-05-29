project('yaml-cpp', 'cpp', license: 'MIT', default_options: [ 'cpp_std=c++17', 'default_library=static', 'warning_level=3' ])

libyaml_cpp = library('yaml_cpp', sources: [
	'src' / 'binary.cpp',
	'src' / 'convert.cpp',
	'src' / 'depthguard.cpp',
	'src' / 'directives.cpp',
	'src' / 'emit.cpp',
	'src' / 'emitfromevents.cpp',
	'src' / 'emitter.cpp',
	'src' / 'emitterstate.cpp',
	'src' / 'emitterutils.cpp',
	'src' / 'exceptions.cpp',
	'src' / 'exp.cpp',
	'src' / 'fptostring.cpp',
	'src' / 'memory.cpp',
	'src' / 'node_data.cpp',
	'src' / 'node.cpp',
	'src' / 'nodebuilder.cpp',
	'src' / 'nodeevents.cpp',
	'src' / 'null.cpp',
	'src' / 'ostream_wrapper.cpp',
	'src' / 'parse.cpp',
	'src' / 'parser.cpp',
	'src' / 'regex_yaml.cpp',
	'src' / 'scanner.cpp',
	'src' / 'scanscalar.cpp',
	'src' / 'scantag.cpp',
	'src' / 'scantoken.cpp',
	'src' / 'simplekey.cpp',
	'src' / 'singledocparser.cpp',
	'src' / 'stream.cpp',
	'src' / 'tag.cpp'
], include_directories: 'include', cpp_args: [
	'-pedantic', '-pedantic-errors', '-DYAML_CPP_NO_CONTRIB',
	(get_option('default_library') == 'static' ? '-DYAML_CPP_STATIC_DEFINE' : '')
])

yaml_dep = declare_dependency(link_with: libyaml_cpp, include_directories: 'include')