# Get simple dependencies
deps = [yaml_cpp, er_cpp]
deps += subproject('dr_libs', required: true).get_variable('dr_libs_dep')
deps += subproject('stb', required: true).get_variable('stb_dep')

# Audio libraries
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
	'CMAKE_POLICY_DEFAULT_CMP0091': 'NEW'
})
audio_sp = cmake.subproject('audiolibs', options: audio_opts, required: true)
ogg = audio_sp.dependency('ogg')
vorbis = audio_sp.dependency('vorbis')
vorbisfile = audio_sp.dependency('vorbisfile')
opus = audio_sp.dependency('opus')
opusfile = audio_sp.dependency('opusfile')
deps += declare_dependency(dependencies: [ogg, vorbis, vorbisfile, opus, opusfile])

# bzip2
bz2_opts = cmake.subproject_options()
bz2_opts.add_cmake_defines({
    'BUILD_SHARED_LIBS': 'OFF',
    'ENABLE_STATIC_LIB': 'ON',
    'ENABLE_SHARED_LIB': 'OFF',
    'ENABLE_LIB_ONLY': 'ON',
    'ENABLE_STATIC_LIB_IS_PIC': 'ON',
    'ENABLE_TESTS': 'OFF',
    'ENABLE_DOCS': 'OFF',
    'CMAKE_POSITION_INDEPENDENT_CODE': 'ON',
	'CMAKE_BUILD_TYPE': cmake_build_type,
	'CMAKE_MSVC_RUNTIME_LIBRARY': cmake_msvc_lib,
	'CMAKE_POLICY_DEFAULT_CMP0091': 'NEW'
})
bz2_sp = cmake.subproject('bzip2', options: bz2_opts, required: true)
deps += bz2_sp.dependency('bz2_static')

formats_lib = static_library('cacaoformats', sources: [
	'src/AudioDecode.cpp',
	'src/ImageOperations.cpp',
    'src/UnpackedDecode.cpp',
	'src/UnpackedEncode.cpp',
    'src/PackedContainer.cpp'
], include_directories: ['include', 'src'], pic: true, dependencies: deps)

formats_dep = declare_dependency(include_directories: 'include', link_with: formats_lib)