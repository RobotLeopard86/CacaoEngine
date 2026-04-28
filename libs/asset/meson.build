asset_deps = [commonlib_dep]

# libjaguar
jaguar_sp = subproject('jaguar', required: true, default_options: ['default_library=static'])
asset_deps += jaguar_sp.get_variable('libjaguar_dep')

# The library itself
asset_lib = static_library('cacaoasset', sources: [
	'src' / 'AssetPack.cpp'
], include_directories: ['include', 'src'], pic: true, dependencies: asset_deps, install: true)