# Pack a0
a0_pak = custom_target('sandbox_a0', output: 'a0.xak', input: ['a0.aam', skybox],
	console: true, command: [xak, '-V', 'create', '-a', meson.current_build_dir() / '..' / 'assets', '-M', '@INPUT0@', '-o', '@OUTPUT@'], build_by_default: true)