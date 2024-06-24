cmake = import('cmake')

yaml_opts = cmake.subproject_options()
yaml_opts.add_cmake_defines({
	'BUILD_SHARED_LIBS': 'OFF',
	'CMAKE_INSTALL_PREFIX': meson.project_build_root() / '.cmakeinstall',
	'YAML_CPP_BUILD_TOOLS': 'OFF',
	'YAML_CPP_BUILD_TESTS': 'OFF',
	'CMAKE_POSITION_INDEPENDENT_CODE': 'ON'
})
yaml_opts.set_install(true)
yaml_dep = cmake.subproject('yaml', options: yaml_opts, required: true).dependency('yaml-cpp')

assimp_opts = cmake.subproject_options()
assimp_opts.add_cmake_defines({
	'BUILD_SHARED_LIBS': 'OFF',
	'CMAKE_INSTALL_PREFIX': meson.project_build_root() / '.cmakeinstall',
	'ASSIMP_BUILD_ZLIB': 'ON',
	'ASSIMP_NO_EXPORT': 'ON',
	'ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT': 'ON',
	'ASSIMP_BUILD_SAMPLES': 'OFF',
	'ASSIMP_BUILD_TESTS': 'OFF',
})
assimp_opts.set_install(true)

assimp_sp = cmake.subproject('assimp', options: assimp_opts, required: true)
assimp_dep = assimp_sp.dependency('assimp')
zlib_dep = assimp_sp.dependency('zlibstatic')

spv_opts = cmake.subproject_options()
spv_opts.add_cmake_defines({
	'CMAKE_INSTALL_PREFIX': meson.project_build_root() / '.cmakeinstall',
	'SPIRV_CROSS_STATIC': 'ON',
	'SPIRV_CROSS_SHARED': 'OFF',
	'SPIRV_CROSS_CLI': 'OFF',
	'CMAKE_POSITION_INDEPENDENT_CODE': 'ON'
})
spv_sp = cmake.subproject('spirv-cross', options: spv_opts, required: true)
spv_core = spv_sp.dependency('spirv-cross-core')
spv_c = spv_sp.dependency('spirv-cross-c')
spv_cpp = spv_sp.dependency('spirv-cross-cpp')
spv_rfl = spv_sp.dependency('spirv-cross-reflect')
spv_util = spv_sp.dependency('spirv-cross-util')
spv_glsl = spv_sp.dependency('spirv-cross-glsl')
spv_hlsl = spv_sp.dependency('spirv-cross-hlsl')
spv_msl = spv_sp.dependency('spirv-cross-msl')

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
    'CMAKE_POSITION_INDEPENDENT_CODE': 'ON'
})
audio_sp = cmake.subproject('audiolibs', options: audio_opts, required: true)
ogg = audio_sp.dependency('ogg')
vorbis = audio_sp.dependency('vorbis')
vorbisfile = audio_sp.dependency('vorbisfile')
opus = audio_sp.dependency('opus')
opusfile = audio_sp.dependency('opusfile')

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
	'CMAKE_POSITION_INDEPENDENT_CODE': 'ON'
})
alsoft_dep = cmake.subproject('openal-soft', options: alsoft_opts, required: true).dependency('OpenAL')

includes = [
	'include',
	'../libs/spdlog/include',
	'../libs/thread-pool/include',
	'../libs/dynalo/include',
	'../libs/glm',
	'../libs/uuid_v4',
	'../libs/dr_libs'
]

cxx = meson.get_compiler('cpp')
libm = cxx.find_library('m', required: false)
libdl = cxx.find_library('dl', required: false)

libcacao = static_library('cacaofrontend', build_by_default: true, sources: [
	'src/Core/Log.cpp',
	'src/Core/Assert.cpp',
	'src/Core/Engine.cpp',
	'src/Events/EventManager.cpp',
	'src/Utilities/Input.cpp',
	'src/3D/Model.cpp',
	'src/3D/Transform.cpp',
	'src/Cameras/PerspectiveCamera.cpp',
	'src/World/WorldManager.cpp',
	'src/Core/DynTickController.cpp',
	'src/Rendering/RenderController.cpp',
	'src/Utilities/AssetManager.cpp',
	'src/Audio/AudioSystem.cpp',
	'src/Audio/Sound.cpp',
	'src/Audio/AudioPlayer.cpp'
], include_directories: includes, dependencies: [
	yaml_dep,
	assimp_dep,
	zlib_dep,
	libm,
	libdl,
	spv_core,
	spv_c,
	spv_cpp,
	spv_util,
	spv_rfl,
	spv_glsl,
	spv_hlsl,
	spv_msl,
	skyshaders_dep,
	alsoft_dep,
	ogg,
	vorbis,
	vorbisfile,
	opus,
	opusfile
], pic: true)

python = find_program('python')
need_pipewire = run_command([ python, meson.global_source_root() / 'scripts' / 'openal_checkpipewire.py'], check: false).returncode()
exe_deps = []
if need_pipewire == 0
	find_program('pkg-config', required: true)
	exe_deps += dependency('libpipewire-0.3', method: 'pkg-config')
endif

cacao_exe = executable('cacaoengine', 'src/Core/Startup.cpp', build_by_default: true, win_subsystem: subsystem, include_directories: includes, 
	build_rpath: '.', link_with: [ libcacao, libbackend ], export_dynamic: true, dependencies: exe_deps)

subdir_done()
