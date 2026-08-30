# Skybox cubemap
skybox = custom_target('skybox.xjc', output: 'skybox.xjc', input: ['skybox.ajc', 'sky_left.png', 'sky_right.png', 'sky_top.png', 'sky_bottom.png', 'sky_front.png', 'sky_back.png'],
	console: true, command: [cubetool, '-V', 'create', '@INPUT0@', '-o', '@OUTPUT@'], build_by_default: true)

# Cube material
cube_mat = custom_target('cubey.xjm', output: 'cubey.xjm', input: 'cubey.ajm', console: true, build_by_default: true, command: [matc, '-V', '@INPUT@', '-o', '@OUTPUT@'])

# Chords music
chords = fs.copyfile('chords.opus')