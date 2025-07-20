# Get simple dependencies
formats_deps = [yaml_cpp, crossguid]
formats_deps += subproject('stb', required: true).get_variable('stb_dep')

# bzip2
bz2_sp = subproject('bzip2', required: true, default_options: ['default_library=static'])
formats_deps += bz2_sp.get_variable('bzip2_dep')

# libarchive
libarchive_sp = subproject('libarchive', required: true, default_options: {
    'default_library': 'static',
    'progs': [],
    'xattr': 'disabled',
    'acl': 'disabled',
    'zlib': 'disabled',
    'bz2lib': 'disabled',
    'libb2': 'disabled',
    'iconv': 'disabled',
    'lz4': 'disabled',
    'zstd': 'disabled',
    'lzma': 'disabled',
    'lzo2': 'disabled',
    'openssl': 'disabled',
    'xml2': 'disabled',
    'expat': 'disabled',
    'regex': 'disabled'
})
formats_deps += libarchive_sp.get_variable('libarchive_dep')
formats_deps += commonlib_dep

formats_lib = static_library('cacaoformats', sources: [
	'src' / 'ImageOperations.cpp',
    'src' / 'UnpackedDecode.cpp',
	'src' / 'UnpackedEncode.cpp',
    'src' / 'PackedContainer.cpp',
    'src' / 'PackedDecode.cpp',
	'src' / 'PackedEncode.cpp'
], include_directories: ['include', 'src'], pic: true, dependencies: formats_deps, install: true, install_tag: 'cacao')

formats_dep = declare_dependency(include_directories: 'include', link_with: formats_lib, dependencies: formats_deps)

if testing
    test('readwrite_material_packed', executable('readwrite_material_packed',
        sources: 'test/readwrite_material_packed.cpp',
        dependencies: formats_dep),
        env: ['RESDIR=' + meson.current_source_dir() / 'test' / 'res'], suite: 'libcacaoformats')
    test('readwrite_material_unpacked', executable('readwrite_material_unpacked',
        sources: 'test/readwrite_material_unpacked.cpp',
        dependencies: formats_dep),
        env: ['RESDIR=' + meson.current_source_dir() / 'test' / 'res'], suite: 'libcacaoformats')
    test('readwrite_world_packed', executable('readwrite_world_packed',
        sources: 'test/readwrite_world_packed.cpp',
        dependencies: formats_dep),
        env: ['RESDIR=' + meson.current_source_dir() / 'test' / 'res'], suite: 'libcacaoformats')
    test('readwrite_world_unpacked', executable('readwrite_world_unpacked',
        sources: 'test/readwrite_world_unpacked.cpp',
        dependencies: formats_dep),
        env: ['RESDIR=' + meson.current_source_dir() / 'test' / 'res'], suite: 'libcacaoformats')
endif