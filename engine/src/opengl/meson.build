# Glad OpenGL loader
gladgl_dep = subproject('glad-gl', required: true).get_variable('glad_gl_dep')

# Add base OpenGL module info
engine_srcs += files(
	'OpenGLModule.cpp',
	'GPU.cpp',
	'Commands.cpp',
	'impls' / 'OpenGLMesh.cpp',
	'impls' / 'OpenGLTex2D.cpp',
	'impls' / 'OpenGLCubemap.cpp',
	'impls' / 'OpenGLShader.cpp'
)
engine_inc += include_directories('.')
engine_deps += gladgl_dep
engine_args_objcpp += ['-Wno-deprecated-declarations', '-DGL_SILENCE_DEPRECATION']

# Add platform-specific info
if host_machine.system() == 'windows'
    engine_srcs += files('ctx' / 'WGL.cpp')
	engine_deps += [cpp.find_library('gdi32', required: true), cpp.find_library('opengl32', required: true), subproject('glad-wgl', required: true).get_variable('glad_wgl_dep')]
elif host_machine.system() == 'darwin'
    engine_srcs += files('ctx' / 'MacOS.mm')
elif host_machine.system() == 'linux'
	engine_srcs += files('ctx' / 'EGL.cpp')
	engine_deps += dependency('egl', required: true)
	if linux_windowing.contains('wayland')
		engine_deps += dependency('wayland-egl', required: true)
	endif
else
    error('Unsupported platform for OpenGL module!')
endif