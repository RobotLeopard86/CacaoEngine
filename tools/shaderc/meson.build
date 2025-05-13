# Shader compiler executable
shaderc = executable('ce-shaderc', sources: [
	'main.cpp',
	'compiler.cpp',
	shaderbasemod
], dependencies: [
	cli11_dep,
	spinners_dep,
	slang_dep,
	commonlib_dep,
	formats_dep
], cpp_args: ['-DCACAO_VER="' + meson.project_version() + '"', '-DCOMPILER_VER="1.0.0"'], include_directories: '..', install: true, install_tag: 'cacao')