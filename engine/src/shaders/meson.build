# Render pipeline shaders
vertex_pipeline = custom_target('vertex_pipeline.inc', input: meson.project_source_root() / 'engine' / 'src' / 'shaders' / 'pipeline' / 'vertex_pipeline.slang', output: 'vertex_pipeline.inc', command: [python, meson.project_source_root() / 'scripts' / 'includable.py', '@INPUT@', '@OUTDIR@'])
fragment_pipeline = custom_target('fragment_pipeline.inc', input: meson.project_source_root() / 'engine' / 'src' / 'shaders' / 'pipeline' / 'fragment_pipeline.slang', output: 'fragment_pipeline.inc', command: [python, meson.project_source_root() / 'scripts' / 'includable.py', '@INPUT@', '@OUTDIR@'])

# Built-in shaders
shaders = [
	'skyshader',
	'basic_color',
	'basic_texture'
]
foreach shader : shaders
	compiled = custom_target(shader + '.xjs', input: shader + '.slang', output: shader + '.xjs', command: [shaderc, '-V', '@INPUT@', '-o', '@OUTPUT@'], build_by_default: true, console: true)
	default_assets += custom_target(shader + '.inc', input: compiled, output: shader + '.inc', command: [python, meson.project_source_root() / 'scripts' / 'arrify.py', '@INPUT@', '@OUTPUT@', shader], build_by_default: true)
endforeach