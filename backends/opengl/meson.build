glad_dep = subproject('glad', required: true).get_variable('glad_dep')
stb_dep = subproject('stb', required: true).get_variable('stb_dep')

include_dirs = [
	'../../cacao/include',
	'include',
	'../common',
	include_directories('../../cacao'),
]

backend_deplist = [
	spv_core,
	spv_c,
	spv_cpp,
	spv_util,
	spv_rfl,
	spv_glsl,
	spv_hlsl,
	spv_msl,
	sdl3,
	glad_dep,
	stb_dep,
	freetype,
	icu,
	harfbuzz,
	harfbuzz_icu,
	harfbuzz_sub,
	core_shaders,
	glm_dep,
	dr_libs_dep,
	dynalo_dep,
	spdlog_dep,
	thread_pool_dep,
	yaml_dep,
	alsoft_dep
]

if host_machine.system() == 'windows'
	backend_deplist += meson.get_compiler('cpp').find_library('imm32', required: true)
endif

libbackend = static_library(
	'cacaobackend',
	include_directories: include_dirs,
	sources: [
		'src/WindowHooks.cpp',
		'src/Texture2D.cpp',
		'src/Cubemap.cpp',
		'src/Shader.cpp',
		'src/Mesh.cpp',
		'src/Skybox.cpp',
		'src/OpenGL.cpp',
		'src/UIDrawing.cpp',
		'src/UIView.cpp',
		'src/Material.cpp',
		'../common/UIShaderGen.cpp',
	],
	dependencies: backend_deplist,
)

subdir_done()