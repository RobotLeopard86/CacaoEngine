# Audio libraries
ogg = subproject('ogg', required: true, default_options: ['default_library=static']).get_variable('libogg_dep')
vorbis_sp= subproject('vorbis', required: true, default_options: ['default_library=static'])
vorbis = vorbis_sp.get_variable('vorbis_dep')
vorbisfile = vorbis_sp.get_variable('vorbisfile_dep')
opus = subproject('opus', default_options: {
	'default_library': 'static',
	'tests': 'disabled',
	'docs': 'disabled',
	'dred': 'enabled',
	'osce': 'enabled',
	'deep-plc': 'enabled',
	'dnn-debug-float': 'disabled',
	'custom-modes': true,
	'extra-programs': 'disabled'
}).get_variable('opus_dep')
opusfile = subproject('opusfile', required: true, default_options: ['default_library=static']).get_variable('opusfile_dep')

# dr_libs
dr_libs_sp = subproject('dr_libs', required: true)
dr_libs_dep = dr_libs_sp.get_variable('dr_libs_dep')

audiodecode_deps = [ogg, vorbis, vorbisfile, opus, opusfile, dr_libs_dep, commonlib_dep]

audiodecode_lib = static_library('cacaoaudiodecode', 'src/AudioDecode.cpp', include_directories: ['include', 'src'], pic: true, dependencies: audiodecode_deps, install: true, install_tag: 'cacao')

audiodecode_dep = declare_dependency(include_directories: 'include', link_with: audiodecode_lib, dependencies: audiodecode_deps)