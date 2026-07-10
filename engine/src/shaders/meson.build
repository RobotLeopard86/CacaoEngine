# Render pipeline shaders
vertex_pipeline = custom_target('vertex_pipeline.inc', input: meson.project_source_root() / 'engine' / 'src' / 'shaders' / 'pipeline' / 'vertex_pipeline.slang', output: 'vertex_pipeline.inc', command: [python, meson.project_source_root() / 'scripts' / 'includable.py', '@INPUT@', '@OUTDIR@'])
fragment_pipeline = custom_target('fragment_pipeline.inc', input: meson.project_source_root() / 'engine' / 'src' / 'shaders' / 'pipeline' / 'fragment_pipeline.slang', output: 'fragment_pipeline.inc', command: [python, meson.project_source_root() / 'scripts' / 'includable.py', '@INPUT@', '@OUTDIR@'])

# Built-in shaders
skyshader = custom_target('skyshader.xjs', input: 'skyshader.slang', output: 'skyshader.xjs', command: [shaderc, '-V', '@INPUT@', '-o', '@OUTPUT@'], build_by_default: true)
default_assets += custom_target('skyshader.inc', input: skyshader, output: 'skyshader.inc', command: [python, meson.project_source_root() / 'scripts' / 'arrify.py', '@INPUT@', '@OUTPUT@', 'skyshader'], build_by_default: true)