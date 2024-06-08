cmake = import('cmake')

glad_dep = subproject('glad', required: true, default_options: [ 'what=gles' ]).get_variable('glad_dep')
stb_dep = subproject('stb', required: true).get_variable('stb_dep')

spv_opts = cmake.subproject_options()
spv_opts.add_cmake_defines({
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

glfw_opts = cmake.subproject_options()
glfw_opts.add_cmake_defines({
	'BUILD_SHARED_LIBS': 'ON',
	'GLFW_BUILD_TESTS': 'OFF',
	'GLFW_BUILD_DOCS': 'OFF',
	'GLFW_BUILD_EXAMPLES': 'OFF'
})
if host_machine.system() == 'linux'
	glfw_opts.add_cmake_defines({
		'GLFW_BUILD_WAYLAND': 'ON',
		'GLFW_BUILD_X11': 'ON'
	})
endif
glfw_dep = cmake.subproject('glfw', options: glfw_opts, required: true).dependency('glfw')

albundle_opts = cmake.subproject_options()
albundle_opts.add_cmake_defines({
	'CMAKE_POSITION_INDEPENDENT_CODE': 'ON',
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
	'ALURE_BUILD_SHARED': 'OFF',
	'ALURE_BUILD_STATIC': 'ON',
	'ALURE_ENABLE_OPUS': 'OFF',
	'ALURE_ENABLE_SNDFILE': 'OFF',
	'ALURE_ENABLE_VORBIS': 'ON',
	'ALURE_INSTALL': 'OFF',
	'ALURE_BUILD_EXAMPLES': 'OFF'
})
albundle = cmake.subproject('albundle', options: albundle_opts, required: true)
vorbisfile_dep = albundle.dependency('vorbisfile')
vorbis_dep = albundle.dependency('vorbis')
ogg_dep = albundle.dependency('ogg')
alsoft_dep = albundle.dependency('OpenAL')
alure_dep = albundle.dependency('alure2_s')

include_dirs = [
	'../../cacao/include',
	'include',
	'../../libs/spdlog/include',
	'../../libs/dynalo/include',
	'../../libs/bsthreadpool',
	'../../libs/glm',
	'../../libs/uuid_v4',
	include_directories('../../cacao')
]

libbackend = shared_library('cacaobackend', include_directories: include_dirs, sources: [
	'src/GLES.cpp',
	'src/Window.cpp',
	'src/Texture2D.cpp',
	'src/Cubemap.cpp',
	'src/Shader.cpp',
	'src/Mesh.cpp',
	'src/Skybox.cpp',
	'src/Audio.cpp',
	'src/Sound.cpp',
	'src/AudioPlayer.cpp',
	'src/ExceptionCodes.cpp'
], dependencies: [
	spv_core,
	spv_c,
	spv_cpp,
	spv_util,
	spv_rfl,
	spv_glsl,
	spv_hlsl,
	spv_msl,
	glfw_dep,
	vorbisfile_dep,
	vorbis_dep,
	ogg_dep,
	alsoft_dep,
	alure_dep,
	glad_dep,
	stb_dep,
	skyshaders_dep
], override_options: [ 'b_lundef=false' ])

ninja = find_program('ninja')
run_command([ ninja, '-C', meson.project_build_root()/'subprojects/albundle/__CMake_build', 'openal-soft/CMakeFiles/build_version'], check: true)

subdir_done()