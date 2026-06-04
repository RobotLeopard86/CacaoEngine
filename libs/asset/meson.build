asset_deps = [commonlib_dep, image_dep, astra_dep]

# libjaguar
jaguar_sp = subproject('jaguar', required: true, default_options: ['default_library=static'])
asset_deps += jaguar_sp.get_variable('libjaguar_dep')

# The library itself
asset_lib = static_library('cacaoasset', sources: [
	'src' / 'AssetPack.cpp',
	'src' / 'Cubemap.cpp',
	'src' / 'Resource.cpp',
	'src' / 'Shader.cpp',
	'src' / 'World.cpp'
], include_directories: ['include', 'src'], pic: true, dependencies: asset_deps, install: true)

# Dependency
asset_dep = declare_dependency(include_directories: 'include', link_with: asset_lib, dependencies: asset_deps)