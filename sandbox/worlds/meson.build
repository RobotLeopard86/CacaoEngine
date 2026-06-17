# Sandbox world
sandbox_world = custom_target('sandbox_world', output: 'sandbox.xjw', input: ['sandbox.ajw'],
	console: true, command: [worldc, '-V', '@INPUT@', '-o', '@OUTPUT@'], build_by_default: true)