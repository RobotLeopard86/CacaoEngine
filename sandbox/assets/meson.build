# Skybox cubemap
skybox = custom_target('sandbox_skybox', output: 'skybox.xjc', input: ['skybox.ajc', 'sky_left.png', 'sky_right.png', 'sky_top.png', 'sky_bottom.png', 'sky_front.png', 'sky_back.png'],
	console: true, command: [cubetool, '-V', 'create', '@INPUT0@', '-o', '@OUTPUT@'], build_by_default: true)