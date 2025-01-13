# Configure Glad (OpenGL loader)
backend_deps += subproject('glad', required: true).get_variable('glad_dep')

# Add sources
src = [
	'Cubemap.cpp',
	'Material.cpp',
	'Mesh.cpp',
	'OpenGL.cpp',
	'Shader.cpp',
	'Skybox.cpp',
	'Texture2D.cpp',
	'UIDrawing.cpp',
	'UIView.cpp',
	'WindowHooks.cpp'
]
foreach s : src
	backend_sources += 'backends' / 'opengl' / s
endforeach