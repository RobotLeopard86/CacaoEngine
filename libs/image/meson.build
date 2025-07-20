# Dependencies
image_deps = []

# Configure KTX
ktx_opts = cmake.subproject_options()
ktx_opts.add_cmake_defines({
	'BUILD_SHARED_LIBS': 'OFF',
	'BUILD_TESTING': 'OFF',
	'ASTCENC_ISA_NONE': 'ON',
	'KTX_FEATURE_GL_UPLOAD': 'OFF',
	'KTX_FEATURE_KTX1': 'OFF',
	'KTX_FEATURE_KTX2': 'ON',
	'KTX_FEATURE_TESTS': 'OFF',
	'KTX_FEATURE_TOOLS': 'OFF',
	'KTX_FEATURE_TOOLS_CTS': 'OFF',
	'KTX_FEATURE_VK_UPLOAD': 'OFF',
    'CMAKE_POSITION_INDEPENDENT_CODE': 'ON',
	'CMAKE_BUILD_TYPE': cmake_build_type,
	'CMAKE_MSVC_RUNTIME_LIBRARY': cmake_msvc_lib,
	'CMAKE_POLICY_DEFAULT_CMP0091': 'NEW'
})
ktx_opts.set_install(false)
ktx_sp = cmake.subproject('ktx', options: ktx_opts, required: true)
image_deps += ktx_sp.dependency('ktx')
image_deps += ktx_sp.dependency('astcenc-none-static')

# libpng
image_deps += subproject('libpng', required: true, default_options: ['default_library=static']).get_variable('libpng_dep')

# libjpeg-turbo
image_deps += subproject('libjpeg-turbo', required: true, default_options: {
	'default_library': 'static',
	'force_inline': false,
	'jpeg-turbo': 'enabled',
	'simd': 'auto',
	'tests': 'disabled',
	'neon-intrinsics': 'auto'
}).get_variable('turbojpeg_dep')

# libtiff
image_deps += subproject('libtiff', required: true, default_options: {
	'default_library': 'static',
	'jbig': 'disabled',
	'jpeg': 'disabled',
	'lerc': 'disabled',
	'lzma': 'disabled',
	'webp': 'disabled',
	'zlib': 'disabled',
	'zstd': 'disabled'
}).get_variable('libtiff4_dep')

# libwebp
image_deps += subproject('libwebp', required: true, default_options: {
	'default_library': 'static',
	'libsharpyuv': 'enabled',
	'libwebp': 'enabled',
	'libwebpmux': 'disabled',
	'libwebpdemux': 'disabled',
	'libwebpdecoder': 'disabled',
	'cwebp': 'disabled',
	'dwebp': 'disabled',
	'webpinfo': 'disabled',
	'webpmux': 'disabled',
	'simd': 'auto',
	'threads': 'enabled',
	'near-lossless': true
}).get_variable('webp_dep')

# tga
image_deps += subproject('tga', required: true).get_variable('tga_dep')

image_lib = static_library('cacaoimage', sources: [
	'src' / 'Forwarding.cpp',
	'src' / 'JPEG.cpp',
	'src' / 'KTX.cpp',
	'src' / 'PNG.cpp',
	'src' / 'TGA.cpp',
	'src' / 'TIFF.cpp',
	'src' / 'WebP.cpp'
], include_directories: ['include', 'src'], pic: true, dependencies: image_deps, install: true, install_tag: 'cacao')

image_dep = declare_dependency(include_directories: 'include', link_with: image_lib, dependencies: image_deps)