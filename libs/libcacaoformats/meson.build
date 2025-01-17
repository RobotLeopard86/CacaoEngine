# Get mode
formats_mode = get_option('formats_mode')

# Define source lists
packed_src = []
unpacked_src = []
common_src = [
	'src/AudioDecode.cpp'
]

# Get simple dependencies
deps = [glm]
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

# Assimp "full-fat" mode
assimp_full_opts = cmake.subproject_options()
assimp_full_opts.add_cmake_defines({
	'BUILD_SHARED_LIBS': 'OFF',
	'CMAKE_INSTALL_PREFIX': meson.project_build_root() / '.cmakeinstall',
	'ASSIMP_BUILD_ZLIB': 'ON',
	'ASSIMP_NO_EXPORT': 'OFF',
	'ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT': 'ON',
	'ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT': 'OFF',
	'ASSIMP_BUILD_DAE_EXPORTER': 'ON',
	'ASSIMP_BUILD_SAMPLES': 'OFF',
	'ASSIMP_BUILD_TESTS': 'OFF',
	'CMAKE_BUILD_TYPE': cmake_build_type,
	'CMAKE_MSVC_RUNTIME_LIBRARY': cmake_msvc_lib,
	'CMAKE_POLICY_DEFAULT_CMP0091': 'NEW'
})

# Assimp "mini" mode that only does importing of Collada files (the format used in asset packs)
assimp_mini_opts = cmake.subproject_options()
assimp_mini_opts.add_cmake_defines({
	'BUILD_SHARED_LIBS': 'OFF',
	'CMAKE_INSTALL_PREFIX': meson.project_build_root() / '.cmakeinstall',
	'ASSIMP_BUILD_ZLIB': 'ON',
	'ASSIMP_NO_EXPORT': 'ON',
	'ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT': 'OFF',
	'ASSIMP_BUILD_DAE_IMPORTER': 'ON',
	'ASSIMP_BUILD_SAMPLES': 'OFF',
	'ASSIMP_BUILD_TESTS': 'OFF',
	'CMAKE_BUILD_TYPE': cmake_build_type,
	'CMAKE_MSVC_RUNTIME_LIBRARY': cmake_msvc_lib,
	'CMAKE_POLICY_DEFAULT_CMP0091': 'NEW'
})

assimp_sp = cmake.subproject('assimp', options: formats_mode == 'packed' ? assimp_mini_opts : assimp_full_opts, required: true)
deps += assimp_sp.dependency('assimp')
deps += assimp_sp.dependency('zlibstatic')

# Calculate final sources list
final_formats_src = []
final_formats_src += common_src
if formats_mode == 'packed' or formats_mode == 'both'
	final_formats_src += packed_src
endif
if formats_mode == 'unpacked' or formats_mode == 'both'
	final_formats_src += unpacked_src
	deps += yaml_cpp
endif

formats_lib = static_library('cacaoformats', sources: final_formats_src, include_directories: ['include', 'src'], pic: true, dependencies: deps, cpp_args: ['-DMODE=' + formats_mode])

formats_dep = declare_dependency(include_directories: 'include', link_with: formats_lib)