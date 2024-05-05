cmake = import('cmake')

glad_dep = subproject('glad', required: true, default_options: [ 'what=gl' ]).get_variable('glad_dep')
stb_dep = subproject('stb', required: true).get_variable('stb_dep')

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

glfw_opts = cmake.subproject_options()
glfw_opts.add_cmake_defines({
	'BUILD_SHARED_LIBS': 'ON',
	'CMAKE_INSTALL_PREFIX': meson.project_build_root() / '.cmakeinstall',
	'GLFW_BUILD_TESTS': 'OFF',
	'GLFW_BUILD_DOCSS': 'OFF',
	'GLFW_BUILD_EXAMPLES': 'OFF'
})
if host_machine.system() == 'linux'
	glfw_opts.add_cmake_defines({
		'GLFW_BUILD_WAYLAND': 'ON',
		'GLFW_BUILD_X11': 'ON'
	})
endif
glfw_dep = cmake.subproject('glfw', options: glfw_opts, required: true).dependency('glfw')

cmake_install_dir = meson.project_build_root() / '.cmakeinstall'

ogg_opts = cmake.subproject_options()
ogg_opts.add_cmake_defines({
	'CMAKE_INSTALL_PREFIX': meson.project_build_root() / '.cmakeinstall',
	'CMAKE_POSITION_INDEPENDENT_CODE': 'ON'
})
ogg_dep = cmake.subproject('ogg', options: ogg_opts, required: true).dependency('ogg')
run_command('ninja', 'install', '-C', meson.project_build_root() / 'subprojects/ogg/__CMake_build', check: true)

if host_machine.system() == 'windows'
	run_command('cmd', '/c', 'mkdir', meson.project_build_root() / 'subprojects\\vorbis', check: true)
  	run_command('cmd', '/c', 'xcopy', '/s', '/e', '/q', '/y', meson.project_build_root() / '.cmakeinstall', meson.project_build_root() / 'subprojects\\vorbis\\.cmakeinstall', check: true)
else
	run_command('mkdir', '-p', meson.project_build_root() / 'subprojects/vorbis', check: true)
  	run_command('cp', '-r', meson.project_build_root() / '.cmakeinstall', meson.project_build_root() / 'subprojects/vorbis/.cmakeinstall', check: true)
endif

vorbis_opts = cmake.subproject_options()
vorbis_opts.add_cmake_defines({
	'CMAKE_INSTALL_PREFIX': meson.project_build_root() / 'subprojects/vorbis/.cmakeinstall',
	'CMAKE_POSITION_INDEPENDENT_CODE': 'ON'
})
vorbis_sp = cmake.subproject('vorbis', options: vorbis_opts, required: true)
vorbis_dep = vorbis_sp.dependency('vorbis')
vorbisfile_dep = vorbis_sp.dependency('vorbisfile')
run_command('ninja', 'install', '-C', meson.project_build_root() / 'subprojects/vorbis/__CMake_build', check: true)

if host_machine.system() == 'windows'
  	run_command('cmd', '/c', 'xcopy', '/s', '/e', '/q', '/y', meson.project_build_root() / 'subprojects\\vorbis\\.cmakeinstall', meson.project_build_root() / '.cmakeinstall', check: true)
else
	run_command('cp', '-r', meson.project_build_root() / 'subprojects/vorbis/.cmakeinstall', meson.project_build_root() / '.cmakeinstall', check: true)
endif

alsoft_opts = cmake.subproject_options()
alsoft_opts.add_cmake_defines({
	'CMAKE_INSTALL_PREFIX': meson.project_build_root() / '.cmakeinstall',
	'ALSOFT_DLOPEN': 'OFF',
	'ALSOFT_EXAMPLES': 'OFF',
	'ALSOFT_NO_CONFIG_UTIL': 'ON',
	'ALSOFT_UTILS': 'OFF',
	'ALSOFT_EAX': 'OFF',
	'ALSOFT_RTKIT': 'OFF',
})
alsoft_dep = cmake.subproject('openal-soft', options: alsoft_opts, required: true).dependency('OpenAL')
run_command('ninja', 'install', '-C', meson.project_build_root() / 'subprojects/openal-soft/__CMake_build', check: true)

if host_machine.system() == 'windows'
	run_command('cmd', '/c', 'mkdir', meson.project_build_root() / 'subprojects\\alure', check: true)
  	run_command('cmd', '/c', 'xcopy', '/s', '/e', '/q', '/y', meson.project_build_root() / '.cmakeinstall', meson.project_build_root() / 'subprojects\\alure\\.cmakeinstall', check: true)
else
	run_command('mkdir', '-p', meson.project_build_root() / 'subprojects/alure', check: true)
  	run_command('cp', '-r', meson.project_build_root() / '.cmakeinstall', meson.project_build_root() / 'subprojects/alure/.cmakeinstall', check: true)
endif

alure_opts = cmake.subproject_options()
alure_opts.add_cmake_defines({
	'CMAKE_INSTALL_PREFIX': meson.project_build_root() / 'subprojects/alure/.cmakeinstall',
	'ALURE_BUILD_SHARED': 'OFF',
	'ALURE_BUILD_STATIC': 'ON',
	'ALURE_ENABLE_OPUS': 'OFF',
	'ALURE_ENABLE_SNDFILE': 'OFF',
	'CMAKE_POSITION_INDEPENDENT_CODE': 'ON'
})
alure_dep = cmake.subproject('alure', options: alure_opts, required: true).dependency('alure2_s')
run_command('ninja', 'install', '-C', meson.project_build_root() / 'subprojects/alure/__CMake_build', check: true)

if host_machine.system() == 'windows'
  	run_command('cmd', '/c', 'xcopy', '/s', '/e', '/q', '/y', meson.project_build_root() / 'subprojects\\alure\\.cmakeinstall', meson.project_build_root() / '.cmakeinstall', check: true)
else
	run_command('cp', '-r', meson.project_build_root() / 'subprojects/alure/.cmakeinstall', meson.project_build_root() / '.cmakeinstall', check: true)
endif

include_dirs = [
	'../../cacao/include',
	'include',
	'../../libs/spdlog/include',
	'../../libs/dynalo/include',
	'../../libs/bsthreadpool',
	'../../libs/glm',
	'../../libs/uuid_v4'
]

libbackend = shared_library('cacaobackend', include_directories: include_dirs, sources: [
	'src/OpenGL.cpp',
	'src/Window.cpp',
	'src/Texture2D.cpp',
	'src/Cubemap.cpp',
	'src/Shader.cpp',
	'src/Mesh.cpp',
	'src/Skybox.cpp'
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
	stb_dep
], override_options: [ 'b_lundef=false' ])

subdir_done()