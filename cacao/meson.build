assimp_opts = cmake.subproject_options()
assimp_opts.add_cmake_defines({
	'BUILD_SHARED_LIBS': 'OFF',
	'CMAKE_INSTALL_PREFIX': meson.project_build_root() / '.cmakeinstall',
	'ASSIMP_BUILD_ZLIB': 'ON',
	'ASSIMP_NO_EXPORT': 'ON',
	'ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT': 'ON',
	'ASSIMP_BUILD_SAMPLES': 'OFF',
	'ASSIMP_BUILD_TESTS': 'OFF',
	'CMAKE_BUILD_TYPE': cmake_build_type,
	'CMAKE_MSVC_RUNTIME_LIBRARY': cmake_msvc_lib,
	'CMAKE_POLICY_DEFAULT_CMP0091': 'NEW'
})
assimp_opts.set_install(true)

assimp_sp = cmake.subproject('assimp', options: assimp_opts, required: true)
assimp_dep = assimp_sp.dependency('assimp')
zlib_dep = assimp_sp.dependency('zlibstatic')

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
alsoft_dep = cmake.subproject('openal-soft', options: alsoft_opts, required: true).dependency('OpenAL')

guid_opts = cmake.subproject_options()
guid_opts.add_cmake_defines({
	'BUILD_SHARED_LIBS': 'OFF',
	'CMAKE_POSITION_INDEPENDENT_CODE': 'ON',
	'CROSSGUID_TESTS': 'OFF',
	'CMAKE_BUILD_TYPE': cmake_build_type,
	'CMAKE_MSVC_RUNTIME_LIBRARY': cmake_msvc_lib,
	'CMAKE_POLICY_DEFAULT_CMP0091': 'NEW'
})
if meson.get_compiler('cpp').get_id() == 'msvc'
	guid_opts.append_compile_args('cpp', '/FI', 'stdint.h')
else
	guid_opts.append_compile_args('cpp', '-include', 'stdint.h')
endif
guid_dep = cmake.subproject('guid', options: guid_opts, required: true).dependency('crossguid')

cxx = meson.get_compiler('cpp')
libdl = cxx.find_library('dl', required: false, static: true)

sources = [
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
	'src/Audio/AudioPlayer.cpp',
	'src/UI/FreetypeOwner.cpp',
	'src/UI/Font.cpp',
	'src/UI/Screen.cpp',
	'src/UI/Renderables.cpp',
	'src/UI/UIView.cpp',
	'expf.c'
]

frontend_deps = [
	yaml_dep,
	assimp_dep,
	zlib_dep,
	libdl,
	core_shaders,
	alsoft_dep,
	ogg,
	vorbis,
	vorbisfile,
	opus,
	opusfile,
	guid_dep,
	spv_core,
	spv_c,
	spv_cpp,
	spv_util,
	spv_rfl,
	spv_glsl,
	spv_hlsl,
	spv_msl,
	freetype,
	icu,
	harfbuzz,
	harfbuzz_icu,
	harfbuzz_sub,
	glm_dep,
	dr_libs_dep,
	thread_pool_dep,
	dynalo_dep,
	spdlog_dep
]
libfrontend = static_library('cacaofrontend', sources: sources, include_directories: 'include', dependencies: frontend_deps, pic: true)

exe_deps = [
	frontend_deps,
	meson.get_compiler('cpp').find_library('atomic', required: host_machine.system() == 'linux')
]

need_pipewire = run_command([ python, meson.project_source_root() / 'scripts' / 'openal_checkpipewire.py'], check: false).returncode()
if need_pipewire == 0
	find_program('pkg-config', required: true)
	exe_deps += dependency('libpipewire-0.3', method: 'pkg-config')
endif

if host_machine.system() == 'windows'
	libcacaoengine = static_library('engine', link_whole: [ libfrontend, libbackend ])
	vs_def = custom_target('genvs_def', input: libcacaoengine, output: 'cacaoengine.def', command: [ python, meson.project_source_root() / 'scripts' / 'symdump.py', '@INPUT@', '@OUTPUT@' ], depends: libcacaoengine)
	lnk_args = []
	if meson.get_compiler('cpp').get_id() == 'msvc'
		lnk_args += '/IGNORE:4102'
	endif
	if subsystem == 'windows'
		lnk_args += meson.get_compiler('cpp').get_linker_id().contains('link') ? '/ENTRY:mainCRTStartup' : ['--entry', 'mainCRTStartup']
	endif
	exe_deps += meson.get_compiler('cpp').find_library('imm32', required: true)
	exe_deps += meson.get_compiler('cpp').find_library('winmm', required: true)
	exe_deps += meson.get_compiler('cpp').find_library('version', required: true)
	exe_deps += meson.get_compiler('cpp').find_library('setupapi', required: true)
	cacao_exe = executable(get_option('exe_name'), 'src/Core/Startup.cpp', win_subsystem: subsystem, include_directories: 'include', 
		build_rpath: '.', link_whole: [ libcacaoengine ], export_dynamic: true, implib: true, dependencies: exe_deps, vs_module_defs: vs_def, link_args: lnk_args)
else
	cacao_exe = executable(get_option('exe_name'), 'src/Core/Startup.cpp', include_directories: 'include', 
		build_rpath: '.', link_whole: [ libfrontend, libbackend ], export_dynamic: true, dependencies: exe_deps)
endif

subdir_done()
