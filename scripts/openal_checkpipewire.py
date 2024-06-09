#!/usr/bin/env python

import os
import pathlib

config_path = pathlib.Path(os.environ['MESON_BUILD_ROOT']) / 'subprojects' / 'albundle' / '__CMake_build' / 'openal-soft' / 'config.h'
with open(config_path, 'r') as cfg_h :
	lines = cfg_h.readlines()
	for line in lines:
		if line.find('#define HAVE_PIPEWIRE') != -1:
			exit(0)
		
	
	exit(1)
