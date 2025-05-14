# Sources and dependencies
tgtgen_src = ['Targetgen.cpp', 'Spv.cpp', shaderbasemod]
tgtgen_deps = [slang_dep, commonlib_dep, formats_dep]
tgtgen_args = []
foreach be : backends
	tgtgen_args += '-DBE_' + be.to_upper()
endforeach
if backends.contains('opengl')
	tgtgen_src = 'GLSL.cpp'
	tgtgen_deps += spirv_cross
endif

# Shader final target generator
targetgen = static_library('cacaotgtgen', include_directories: 'include', sources: tgtgen_src, dependencies: tgtgen_deps, cpp_args: tgtgen_args)

targetgen_dep = declare_dependency(link_whole: targetgen, include_directories: 'include', dependencies: commonlib_dep)