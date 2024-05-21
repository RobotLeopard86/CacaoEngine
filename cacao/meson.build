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
	'CMAKE_POSITION_INDEPENDENT_CODE': 'ON'
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

includes = [
	'include',
	'../libs/spdlog/include',
	'../libs/dynalo/include',
	'../libs/bsthreadpool',
	'../libs/glm',
	'../libs/uuid_v4'
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
	'src/Control/DynTickController.cpp',
	'src/Rendering/RenderController.cpp',
	'src/Utilities/AssetManager.cpp',
	'src/Audio/AudioController.cpp'
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
], pic: true)

executable('cacaoengine', 'src/Core/Startup.cpp', build_by_default: true, win_subsystem: subsystem, include_directories: includes, 
	build_rpath: '.', link_with: [ libcacao, libbackend ], export_dynamic: true)

subdir_done()