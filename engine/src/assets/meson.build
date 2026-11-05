# Generate asset binary includes
default_assets = []
meshes = [
    'capsule', 'cone', 'cube', 'cylinder', 'quad', 'quadpyramid', 'sphere', 'tripyramid'
]
foreach mesh : meshes
    default_assets += custom_target(mesh + '.inc', input: mesh + '.glb', output: mesh + '.inc', command: [python, meson.project_source_root() / 'scripts' / 'arrify.py', '@INPUT@', '@OUTPUT@', mesh], build_by_default: true)
endforeach