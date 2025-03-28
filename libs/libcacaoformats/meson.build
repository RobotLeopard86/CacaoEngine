# Get simple dependencies
deps = [yaml_cpp]
deps += subproject('dr_libs', required: true).get_variable('dr_libs_dep')
deps += subproject('stb', required: true).get_variable('stb_dep')

# bzip2
bz2_sp = subproject('bzip2', required: true, default_options: ['default_library=static'])
deps += bz2_sp.get_variable('bzip2_dep')

# libarchive
libarchive_sp = subproject('libarchive', required: true, default_options: {
    'default_library': 'static',
    'progs': [],
    'xattr': 'disabled',
    'acl': 'disabled',
    'zlib': 'disabled',
    'bz2lib': 'disabled',
    'libb2': 'disabled',
    'iconv': 'disabled',
    'lz4': 'disabled',
    'zstd': 'disabled',
    'lzma': 'disabled',
    'lzo2': 'disabled',
    'openssl': 'disabled',
    'xml2': 'disabled',
    'expat': 'disabled',
    'regex': 'disabled'
})
deps += libarchive_sp.get_variable('libarchive_dep')

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

formats_lib = static_library('cacaoformats', sources: [
	'src/AudioDecode.cpp',
	'src/ImageOperations.cpp',
    'src/UnpackedDecode.cpp',
	'src/UnpackedEncode.cpp',
    'src/PackedContainer.cpp',
    'src/PackedDecode.cpp',
	'src/PackedEncode.cpp'
], include_directories: ['include', 'src'], pic: true, dependencies: deps)

formats_dep = declare_dependency(include_directories: 'include', link_with: formats_lib)

if testing
    test('readwrite_material_packed', executable('readwrite_material_packed',
        sources: 'test/readwrite_material_packed.cpp',
        dependencies: formats_dep),
        env: ['RESDIR=' + meson.current_source_dir() / 'test' / 'res'], suite: 'libcacaoformats')
    test('readwrite_material_unpacked', executable('readwrite_material_unpacked',
        sources: 'test/readwrite_material_unpacked.cpp',
        dependencies: formats_dep),
        env: ['RESDIR=' + meson.current_source_dir() / 'test' / 'res'], suite: 'libcacaoformats')
endif