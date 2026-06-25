# Configure assimp
assimp_sp = subproject('assimp', required: true)
assimp_dep = assimp_sp.get_variable('assimp_dep')

# Configure OpenAL-Soft
alsoft_sp = subproject('openal-soft', required: true, default_options: {
	'default_library': 'shared',
	'utils': false,
	'rtkit': 'disabled',
	'sdl2': 'disabled',
	'winmm': 'disabled'
})
alsoft_dep = alsoft_sp.get_variable('openal_dep')

# Configure spdlog
spdlog_dep = subproject('spdlog', required: true, default_options: {
	'default_library': 'static',
	'tests': 'disabled',
	'compile_library': false,
	'std_format': 'disabled'
}).get_variable('spdlog_dep')

# Exathread, glm, high_resolution_sleep, eternal, and mikktspace
exathread_dep = subproject('exathread', required: true).get_variable('exathread_dep')
glm_dep = subproject('glm', required: true).get_variable('glm_dep')
eternal_dep = subproject('eternal', required: true).get_variable('eternal_dep')
mikktspace_dep = subproject('mikktspace', required: true).get_variable('mikktspace_dep')

# Configure icu
icu_sp = subproject('icu', required: true, default_options: ['default_library=static'])
icu_dep = icu_sp.get_variable('icuuc_dep')

# Configure FreeType
freetype_sp = subproject('freetype2', required: true, default_options: ['default_library=static', 'png=disabled', 'zlib=disabled', 'brotli=disabled', 'bzip2=disabled', 'harfbuzz=disabled'])
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
apple_audio = dependency('appleframeworks', modules: ['CoreAudio', 'AudioToolbox'], required: host_machine.system() == 'darwin')

# Render pipeline shaders
vertex_pipeline = custom_target('cacaoengine-vertexpipeline', input: meson.project_source_root() / 'engine' / 'src' / 'shaders' / 'vertex_pipeline.slang', output: 'vertex_pipeline.inc', command: [python, meson.project_source_root() / 'scripts' / 'includable.py', '@INPUT@', '@OUTDIR@'])
fragment_pipeline = custom_target('cacaoengine-fragmentpipeline', input: meson.project_source_root() / 'engine' / 'src' / 'shaders' / 'fragment_pipeline.slang', output: 'fragment_pipeline.inc', command: [python, meson.project_source_root() / 'scripts' / 'includable.py', '@INPUT@', '@OUTDIR@'])

# Add core module data
engine_srcs += files(
	'expf.c',
	'Actor.cpp',
	'AudioManager.cpp',
	'CodeRegistry.cpp',
	'Cubemap.cpp',
	'Engine.cpp',	
	'EventManager.cpp',
	'FrameProcessor.cpp',
	'GPU.cpp',
	'Input.cpp',
	'Log.cpp',
	'Mesh.cpp',
	'Misc.cpp',
	'Model.cpp',
	'PAL.cpp',
	'PerspectiveCamera.cpp',
	'Resource.cpp',
	'Shader.cpp',
	'Sound.cpp',
	'Targetgen.cpp',
	'Tex2D.cpp',
	'TickController.cpp',
	'Transform.cpp',
	'Window.cpp',
	'World.cpp',
)
engine_srcs += [shaderbasemod, vertex_pipeline, fragment_pipeline]
engine_deps += [
	yaml_cpp,
	astra_dep,
	crossguid,
	spirv_cross,
	audiodecode_dep,
	asset_dep,
	image_dep,
	commonlib_dep,
	assimp_dep,
	alsoft_dep,
	glm_dep,
	spdlog_dep,
	exathread_dep,
	eternal_dep,
	slang_dep,
	icu_dep,
	freetype_dep,
	harfbuzz_dep,
	harfbuzz_icu_dep,
	harfbuzz_sub_dep,
	mikktspace_dep,
	libdl,
	libatomic,
	libpipewire,
	imm32,
	winmm,
	versionlib,
	setupapi,
	apple_audio
]