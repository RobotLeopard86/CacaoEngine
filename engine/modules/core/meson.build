# Configure assimp
assimp_opts = cmake.subproject_options()
assimp_opts.add_cmake_defines({
	'BUILD_SHARED_LIBS': 'OFF',
	'ASSIMP_BUILD_ZLIB': 'ON',
	'ASSIMP_NO_EXPORT': 'ON',
	'ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT': 'OFF',
	'ASSIMP_BUILD_FBX_IMPORTER': 'ON',
	'ASSIMP_BUILD_GLTF_IMPORTER': 'ON',
	'ASSIMP_BUILD_COLLADA_IMPORTER': 'ON',
	'ASSIMP_BUILD_OBJ_IMPORTER': 'ON',
	'ASSIMP_BUILD_SAMPLES': 'OFF',
	'ASSIMP_BUILD_TESTS': 'OFF',
    'CMAKE_POSITION_INDEPENDENT_CODE': 'ON',
	'CMAKE_BUILD_TYPE': cmake_build_type,
	'CMAKE_MSVC_RUNTIME_LIBRARY': cmake_msvc_lib,
	'CMAKE_POLICY_DEFAULT_CMP0091': 'NEW'
})
assimp_opts.set_install(false)
assimp_sp = cmake.subproject('assimp', options: assimp_opts, required: true)
assimp_dep = assimp_sp.dependency('assimp')
zlib_dep = assimp_sp.dependency('zlibstatic')

# Configure OpenAL-Soft
alsoft_opts = cmake.subproject_options()
alsoft_opts.add_cmake_defines({
	'BUILD_TESTING': 'OFF',
	'BUILD_SHARED_LIBS': 'OFF',
	'LIBTYPE': 'STATIC',
	'ALSOFT_DLOPEN': 'OFF',
	'ALSOFT_EXAMPLES': 'OFF',
	'ALSOFT_NO_CONFIG_UTIL': 'ON',
	'ALSOFT_EAX': 'OFF',
	'ALSOFT_UTILS': 'OFF',
	'ALSOFT_RTKIT': 'OFF',
	'ALSOFT_EMBED_HRTF_DATA': 'OFF',
	'ALSOFT_BACKEND_WINMM': 'OFF',
	'CMAKE_POSITION_INDEPENDENT_CODE': 'ON',
	'CMAKE_BUILD_TYPE': cmake_build_type,
	'CMAKE_MSVC_RUNTIME_LIBRARY': cmake_msvc_lib,
	'CMAKE_POLICY_DEFAULT_CMP0091': 'NEW'
})
alsoft_opts.set_install(false)
alsoft_dep = cmake.subproject('openal-soft', options: alsoft_opts, required: true).dependency('OpenAL')

# Configure spdlog
spdlog_opts = cmake.subproject_options()
spdlog_opts.add_cmake_defines({
	'SPDLOG_BUILD_PIC': 'ON',
	'CMAKE_POSITION_INDEPENDENT_CODE': 'ON',
	'CMAKE_BUILD_TYPE': cmake_build_type,
	'CMAKE_MSVC_RUNTIME_LIBRARY': cmake_msvc_lib,
	'CMAKE_POLICY_DEFAULT_CMP0091': 'NEW'
})
spdlog_opts.set_install(false)
spdlog_dep = cmake.subproject('spdlog', options: spdlog_opts, required: true).dependency('spdlog')

# glm, dynalo, and thread-pool
glm_dep = subproject('glm', required: true).get_variable('glm_dep')
dynalo_dep = subproject('dynalo', required: true).get_variable('dynalo_dep')
thread_pool_dep = subproject('thread-pool', required: true).get_variable('thread_pool_dep')

# Configure icu
icu_sp = subproject('icu', required: true, default_options: ['default_library=static'])
icu_dep = icu_sp.get_variable('icuuc_dep')

# Configure FreeType
freetype_sp = subproject('freetype', required: true, default_options: ['default_library=static', 'png=disabled', 'zlib=disabled', 'brotli=disabled', 'bzip2=disabled', 'harfbuzz=disabled'])
freetype_dep = freetype_sp.get_variable('freetype_dep')

# Configure Harfbuzz
harfbuzz_sp = subproject('harfbuzz', required: true, default_options: ['default_library=static', 'freetype=enabled', 'cairo=disabled', 'graphite2=disabled', 'glib=disabled', 'gobject=disabled', 'chafa=disabled', 'icu=enabled', 'gdi=disabled', 'directwrite=disabled', 'coretext=disabled', 'wasm=disabled', 'tests=disabled', 'introspection=disabled', 'docs=disabled', 'utilities=disabled'])
harfbuzz_dep = harfbuzz_sp.get_variable('libharfbuzz_dep')
harfbuzz_icu_dep = harfbuzz_sp.get_variable('libharfbuzz_icu_dep')
harfbuzz_sub_dep = harfbuzz_sp.get_variable('libharfbuzz_subset_dep')

# System libraries
libdl = cpp.find_library('dl', required: host_machine.system() != 'windows', static: true)
libatomic = cpp.find_library('atomic', required: host_machine.system() == 'linux')
libpipewire = dependency('libpipewire-0.3', method: 'pkg-config', required: run_command([ python, meson.project_source_root() / 'scripts' / 'openal_checkpipewire.py'], check: false).returncode() == 0)
imm32 = cpp.find_library('imm32', required: host_machine.system() == 'windows')
winmm = cpp.find_library('winmm', required: host_machine.system() == 'windows')
versionlib = cpp.find_library('version', required: host_machine.system() == 'windows')
setupapi = cpp.find_library('setupapi', required: host_machine.system() == 'windows')

# Dependencies list
core_deps = [
	yaml_cpp,
	silica_dep,
	crossguid,
	spv_core,
	spv_c,
	spv_cpp,
	spv_glsl,
	spv_hlsl,
	spv_msl,
	spv_rfl,
	spv_util,
	audiodecode_dep,
	formats_dep,
	commonlib_dep,
	assimp_dep,
	zlib_dep,
	alsoft_dep,
	glm_dep,
	spdlog_dep,
	dynalo_dep,
	thread_pool_dep,
	icu_dep,
	freetype_dep,
	harfbuzz_dep,
	harfbuzz_icu_dep,
	harfbuzz_sub_dep,
	libdl,
	libatomic,
	libpipewire,
	imm32,
	winmm,
	versionlib,
	setupapi
]

# Core library
libcore = static_library('cacaocore', sources: [
	'src' / 'expf.c',
	'src' / 'AudioManager.cpp',
	'src' / 'Engine.cpp',
	'src' / 'EventManager.cpp',
	'src' / 'Log.cpp',
	'src' / 'PAL.cpp',
	'src' / 'ThreadPool.cpp'
], dependencies: core_deps, include_directories: [engine_headers, module_private_headers], install: true, install_tag: 'cacao',
cpp_args: '-DCACAO_VER="' + meson.project_version() + '"', pic: true)
core_dep = declare_dependency(link_whole: libcore, dependencies: core_deps)