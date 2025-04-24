# CMake-based audio libraries
audio_opts = cmake.subproject_options()
audio_opts.add_cmake_defines({
    'BUILD_SHARED_LIBS': 'OFF',
    'OP_DISABLE_HTTP': 'ON',
   	'OP_DISABLE_EXAMPLES': 'ON',
    'OP_DISABLE_DOCS': 'ON',
    'OPUS_BUILD_SHARED_LIBRARY': 'OFF',
    'OPUS_BUILD_TESTING': 'OFF',
    'OPUS_CUSTOM_MODES': 'ON',
    'OPUS_BUILD_PROGRAMS': 'OFF',
	'OPUS_STATIC_RUNTIME': 'OFF',
    'CMAKE_POSITION_INDEPENDENT_CODE': 'ON',
	'CMAKE_BUILD_TYPE': cmake_build_type,
	'CMAKE_MSVC_RUNTIME_LIBRARY': cmake_msvc_lib,
	'CMAKE_POLICY_DEFAULT_CMP0091': 'NEW',
    'CMAKE_POLICY_DEFAULT_CMP0126': 'NEW',
    'CMAKE_POLICY_VERSION_MINIMUM': '3.5'
})
audio_sp = cmake.subproject('audiolibs', options: audio_opts, required: true)
ogg = audio_sp.dependency('ogg')
vorbis = audio_sp.dependency('vorbis')
vorbisfile = audio_sp.dependency('vorbisfile')
opus = audio_sp.dependency('opus')
opusfile = audio_sp.dependency('opusfile')

# dr_libs
dr_libs_sp = subproject('dr_libs', required: true)
dr_libs_dep = dr_libs_sp.get_variable('dr_libs_dep')

audiodecode_deps = [ogg, vorbis, vorbisfile, opus, opusfile, dr_libs_dep, commonlib_dep]

audiodecode_lib = static_library('cacaoaudiodecode', 'src/AudioDecode.cpp', include_directories: ['include', 'src'], pic: true, dependencies: audiodecode_deps)

audiodecode_dep = declare_dependency(include_directories: 'include', link_with: audiodecode_lib, dependencies: audiodecode_deps)