project('slang', 'c', 'cpp', meson_version: '>=1.2.0', version: '2025.19.1', license: 'Apache-2.0 WITH LLVM-exception', subproject_dir: 'external', default_options: ['cpp_std=c++20', 'warning_level=3', 'b_staticpic=true', 'default_library=static'])

###################################
# COMMON SETUP
###################################

# Common stuff
export_args = [
    '-DSLANG_DYNAMIC',
    '-DSLANG_DYNAMIC_EXPORT',
]
common_includes = include_directories('include', 'source')

# Disable certain common warnings
if meson.get_compiler('cpp').get_argument_syntax() == 'gcc'
    add_project_arguments(['-Wno-assume', '-Wno-reorder-ctor', '-Wno-switch', '-Wno-unused-but-set-variable', '-Wno-unused-parameter', '-Wno-extra-semi', '-Wno-unused-variable', '-Wno-logical-op-parentheses'], language: 'cpp')
elif meson.get_compiler('cpp').get_argument_syntax() == 'msvc'
    add_project_arguments(['/wd26495', '/wd4065', '/wd4189', '/wd4100', '/wd4068', '/wd4189', '/wd4805', '/wd4804'], language: 'cpp')
endif

# Disable weird error on GCC
if meson.get_compiler('cpp').get_id() == 'gcc'
    add_project_arguments('-Wno-error=stringop-overflow', language: 'cpp')
endif

###################################
# SUBPROJECTS
###################################

# Subproject patches
python = find_program('python3')
run_command([python, meson.project_source_root() / '_fixes' / 'patch.py'], check: true)

# unordered_dense
unordered_dense = subproject('unordered_dense')
unordered_dense_dep = unordered_dense.get_variable('unordered_dense_dep')

# miniz
miniz = subproject('miniz', required: true)
miniz_dep = miniz.get_variable('miniz_dependency')

# lz4
lz4 = subproject('lz4')
lz4_dep = lz4.get_variable('liblz4_dep')

# vulkan-headers
vulkan_headers = subproject('vulkan')
vulkan_headers_dep = vulkan_headers.get_variable('vulkan_headers_dep')

# metal-cpp
metal_cpp = subproject('metal-cpp')
metal_cpp_dep = metal_cpp.get_variable('metal_cpp_dep')

# spirv-headers
spirv_headers = subproject('spirv-headers')
spirv_headers_dep = spirv_headers.get_variable('spirv_headers_dep')

# lua
lua = subproject('lua')
lua_headers_dep = lua.get_variable('lua_headers_dep')

# threads
threads = dependency('threads', required: true)

# dl
libdl_dep = meson.get_compiler('cpp').find_library('dl', required: host_machine.system() != 'windows')

###################################
# VERSION FILE
###################################
verfile_config = configuration_data()
verfile_config.set('SLANG_VERSION_FULL', 'v' + meson.project_version())
verfile = configure_file(input: 'slang-tag-version.h.in', output: 'slang-tag-version.h', format: 'cmake@', configuration: verfile_config)

###################################
# CORE LIBRARIES
###################################

# core
core_src = run_command([python, 'sourceglob.py', meson.project_source_root() / 'source' / 'core'], check: true).stdout().strip().splitlines()
core = library('core', sources: core_src, include_directories: common_includes, dependencies: [miniz_dep, lz4_dep, threads, libdl_dep, unordered_dense_dep], cpp_args: export_args)
slang_core_dep = declare_dependency(include_directories: common_includes, dependencies: [unordered_dense_dep], link_with: core)

# compiler-core
compiler_core_src = run_command([python, 'sourceglob.py', meson.project_source_root() / 'source' / 'compiler-core'], check: true).stdout().strip().splitlines()
compiler_core = library('compiler-core', sources: compiler_core_src, include_directories: common_includes, dependencies: [slang_core_dep, spirv_headers_dep], cpp_args: [export_args, '-fms-extensions'])
slang_compiler_core_dep = declare_dependency(dependencies: spirv_headers_dep, link_with: compiler_core)

###################################
# GENERATOR TOOLS
###################################

# slang-fiddle
slang_fiddle_src = run_command([python, 'sourceglob.py', meson.project_source_root() / 'tools' / 'slang-fiddle'], check: true).stdout().strip().splitlines()
slang_fiddle = executable('slang-fiddle', sources: slang_fiddle_src, dependencies: [slang_core_dep, slang_compiler_core_dep, lua_headers_dep], cpp_args: '-Wno-gnu-label-as-value')

# slang-embed
slang_embed_src = run_command([python, 'sourceglob.py', meson.project_source_root() / 'tools' / 'slang-embed'], check: true).stdout().strip().splitlines()
slang_embed = executable('slang-embed', sources: slang_embed_src, dependencies: [slang_core_dep])

# slang-generate
slang_generate_src = run_command([python, 'sourceglob.py', meson.project_source_root() / 'tools' / 'slang-generate'], check: true).stdout().strip().splitlines()
slang_generate = executable('slang-generate', sources: slang_generate_src, dependencies: [slang_core_dep])

# slang-lookup-generator
slang_lookup_generator_src = run_command([python, 'sourceglob.py', meson.project_source_root() / 'tools' / 'slang-lookup-generator'], check: true).stdout().strip().splitlines()
slang_lookup_generator = executable('slang-lookup-generator', sources: slang_lookup_generator_src, dependencies: [slang_core_dep, slang_compiler_core_dep])

# slang-capability-generator
slang_capability_generator_src = run_command([python, 'sourceglob.py', meson.project_source_root() / 'tools' / 'slang-capability-generator'], check: true).stdout().strip().splitlines()
slang_capability_generator = executable('slang-capability-generator', sources: slang_capability_generator_src, dependencies: [slang_core_dep, slang_compiler_core_dep])

# slang-spirv_embed-generator
slang_spirv_embed_generator_src = run_command([python, 'sourceglob.py', meson.project_source_root() / 'tools' / 'slang-spirv-embed-generator'], check: true).stdout().strip().splitlines()
slang_spirv_embed_generator = executable('slang-spirv-embed-generator', sources: slang_spirv_embed_generator_src, dependencies: [slang_core_dep, slang_compiler_core_dep])

###################################
# PRELUDE
###################################

prelude_headers = run_command([python, 'fglob.py', meson.project_source_root() / 'prelude', '.*-prelude\.h'], check: true).stdout().strip().splitlines()
prelude_sources = []
foreach header : prelude_headers
    out = (header.split('/')[-1]).replace('.h', '.cpp')
    prelude_sources += custom_target(out, command: [slang_embed, '@INPUT@', '@OUTPUT@', '-I' + (meson.project_source_root() / 'include'), '-I' + (meson.project_source_root() / 'source')], output: out,
        input: header)
endforeach
slang_prelude_dep = declare_dependency(sources: prelude_sources, include_directories: include_directories('prelude', 'include'), dependencies: [unordered_dense_dep])

###################################
# CORE MODULE HEADER SETUP
###################################

core_module_metaslang_files= run_command([python, 'fglob.py', meson.project_source_root() / 'source' / 'slang', '.*\.meta\.slang'], check: true).stdout().strip().splitlines()
core_module_meta_outputs = []
foreach metaslang : core_module_metaslang_files
    core_module_meta_outputs += (metaslang.split('/')[-1]) + '.h'
endforeach
core_module_meta_headers = custom_target('core_module_meta_headers', command: [slang_generate, '@INPUT@', '--target-directory', meson.current_build_dir()], 
    output: core_module_meta_outputs, input: core_module_metaslang_files)

###################################
# FIDDLE GENERATION
###################################

fiddle_input_files= run_command([python, 'sourceglob.py', meson.project_source_root() / 'source' / 'slang'], check: true).stdout().strip().splitlines()
fiddle_inputs = []
foreach input : fiddle_input_files
    fiddle_inputs += (input.split('/')[-1])
endforeach
fiddle_outputs = []
foreach input : fiddle_inputs
    fiddle_outputs += input + '.fiddle'
endforeach
fiddle_lua = run_command([python, 'fglob.py', meson.project_source_root() / 'source' / 'slang', '.*\.lua'], check: true).stdout().strip().splitlines()
fiddle_output = custom_target('fiddle_output', command: [python, meson.project_source_root() / 'invokewrapper.py', slang_fiddle, '-i', meson.project_source_root() / 'source' / 'slang/', 
    '-o', meson.current_build_dir() + '/', fiddle_inputs], output: fiddle_outputs, input: fiddle_input_files, depend_files: [fiddle_lua, 'invokewrapper.py'])
fiddle_output_dep = declare_dependency(sources: fiddle_output, include_directories: include_directories('.'))

###################################
# CAPABILITY GENERATION
###################################

cap_input_files= run_command([python, 'fglob.py', meson.project_source_root() / 'source' / 'slang', '.*\.capdef'], check: true).stdout().strip().splitlines()
cap_generated_headers = [
    'slang-generated-capability-defs.h',
    'slang-generated-capability-defs-impl.h'
]
cap_generated_sources = [
    'slang-lookup-capability-defs.cpp'
]
foreach cgh : cap_generated_headers
    cap_generated_sources += cgh
endforeach
capgen = custom_target('capability_generation', command: [slang_capability_generator, '@INPUT@', '--target-directory', meson.current_build_dir(), 
    '--doc', meson.project_source_root() / 'docs' / 'user-guide' / 'a3-02-reference-capability-atoms.md'], output: cap_generated_sources, input: cap_input_files)
capability_defs_dep = declare_dependency(sources: [capgen[1], capgen[2]], dependencies: slang_core_dep, include_directories: include_directories('.', 'source' / 'slang'), 
    compile_args: export_args)
capability_lookup_dep = declare_dependency(sources: capgen, dependencies: [slang_core_dep, capability_defs_dep], compile_args: export_args)

###################################
# LOOKUP TABLES GENERATION
###################################

lookup_generated_sources = [
    'slang-lookup-GLSLstd450.cpp'
]
lookupgen = custom_target('lookup_generation', command: [slang_lookup_generator, meson.project_source_root() / 'external' / 'spirv-headers' / 'include' / 'spirv' / 'unified1' / 'extinst.glsl.std.450.grammar.json',
    '@OUTPUT@', 'GLSLstd450', 'GLSLstd450', 'spirv/unified1/GLSL.std.450.h'], output: lookup_generated_sources)
spirv_grammar_source = [
    'slang-spirv-core-grammar-embed.cpp'
]
spirv_grammar = custom_target('spirv_grammar', command: [slang_spirv_embed_generator, meson.project_source_root() / 'external' / 'spirv-headers' / 'include' / 'spirv' / 'unified1' / 'spirv.core.grammar.json', 
    '@OUTPUT@'], output: spirv_grammar_source, )
lookup_tables_dep = declare_dependency(sources: [lookupgen, spirv_grammar], dependencies: [slang_core_dep, spirv_headers_dep], compile_args: export_args)

###################################
# CORE MODULE SOURCE
###################################

embedded_core_module_source_dep = declare_dependency(sources: ['source' / 'slang-core-module' / 'slang-embedded-core-module-source.cpp', core_module_meta_headers], compile_args: [export_args, '-DSLANG_EMBED_CORE_MODULE_SOURCE'], 
    dependencies: [slang_core_dep, capability_defs_dep, fiddle_output_dep, spirv_headers_dep], include_directories: common_includes)
no_embedded_core_module_source_dep = declare_dependency(sources: 'source' / 'slang-core-module' / 'slang-embedded-core-module-source.cpp', compile_args: export_args, dependencies: [slang_core_dep, capability_defs_dep, 
    fiddle_output_dep, spirv_headers_dep], include_directories: common_includes)

###################################
# SLANG COMMON STUFF
###################################

slang_base_src = run_command([python, 'sourceglob.py', meson.project_source_root() / 'source' / 'slang'], check: true).stdout().strip().splitlines()
slang_record_src = run_command([python, 'sourceglob.py', meson.project_source_root() / 'source' / 'slang-record-replay' / 'record'], check: true).stdout().strip().splitlines()
slang_replay_src = run_command([python, 'sourceglob.py', meson.project_source_root() / 'source' / 'slang-record-replay' / 'replay'], check: true).stdout().strip().splitlines()
slang_record_replay_util_src = run_command([python, 'sourceglob.py', meson.project_source_root() / 'source' / 'slang-record-replay' / 'util'], check: true).stdout().strip().splitlines()
slang_src = [slang_base_src, slang_record_src, slang_replay_src, slang_record_replay_util_src]
slang_common_dep = declare_dependency(sources: [slang_src, verfile], compile_args: export_args, include_directories: common_includes, dependencies: [slang_core_dep, slang_prelude_dep, slang_compiler_core_dep,
    capability_defs_dep, capability_lookup_dep, fiddle_output_dep, lookup_tables_dep, spirv_headers_dep])

###################################
# CORE MODULE BOOTSTRAP SETUP
###################################

no_embedded_core_module_dep = declare_dependency(sources: 'source' / 'slang-core-module' / 'slang-embedded-core-module.cpp', dependencies: slang_core_dep, compile_args: export_args)
slang_without_embedded_core_module = library('slang_without_embedded_core_module', dependencies: [slang_core_dep, slang_prelude_dep, slang_compiler_core_dep, capability_defs_dep, capability_lookup_dep,
    fiddle_output_dep, lookup_tables_dep, spirv_headers_dep, slang_common_dep, embedded_core_module_source_dep, no_embedded_core_module_dep], include_directories: common_includes)
slang_bootstrap_src = run_command([python, 'sourceglob.py', meson.project_source_root() / 'source' / 'slangc'], check: true).stdout().strip().splitlines()
slang_bootstrap = executable('slang_bootstrap', sources: slang_bootstrap_src, cpp_args: '-DSLANG_BOOTSTRAP=1', link_with: slang_without_embedded_core_module, dependencies: [slang_prelude_dep, capability_lookup_dep,
    lookup_tables_dep, threads, slang_core_dep])

###################################
# CORE MODULE BOOTSTRAPPING
###################################

core_module_generated_headers = [
    'slang-core-module-generated.h'
]
glsl_module_generated_headers = [
    'slang-glsl-module-generated.h'
]
bootstrap_headers = custom_target('bootstrap_headers', command: [slang_bootstrap, '-archive-type', 'riff-lz4', '-save-core-module-bin-source', '@OUTPUT0@', '-save-glsl-module-bin-source', '@OUTPUT1@'], 
    output: [core_module_generated_headers, glsl_module_generated_headers], depends: slang_without_embedded_core_module)
embedded_core_module_dep = declare_dependency(sources: ['source' / 'slang-core-module' / 'slang-embedded-core-module.cpp', bootstrap_headers], dependencies: slang_core_dep, compile_args: [export_args, 
    '-DSLANG_EMBED_CORE_MODULE_SOURCE'])
glsl_module = shared_module('slang_glsl_module', sources: [bootstrap_headers[1], 'source' / 'slang-glsl-module' / 'slang-embedded-glsl-module.cpp'], cpp_args: '-DSLANG_SHARED_LIBRARY_TOOL',
    gnu_symbol_visibility: 'hidden', dependencies: slang_core_dep)

###################################
# FINAL SLANG LIBRARY
###################################

slang = library('slang', dependencies: [slang_core_dep, slang_prelude_dep, slang_compiler_core_dep, capability_defs_dep, capability_lookup_dep,
    fiddle_output_dep, lookup_tables_dep, spirv_headers_dep, slang_common_dep, embedded_core_module_dep, embedded_core_module_source_dep], include_directories: common_includes)
slang_dep = declare_dependency(include_directories: common_includes, sources: verfile, link_with: slang)