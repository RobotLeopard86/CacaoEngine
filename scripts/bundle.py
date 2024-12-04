#!/usr/bin/env python

import pathlib
import shutil
import sys
import os

buildroot = pathlib.Path(os.environ['MESON_BUILD_ROOT'])

if not buildroot.exists():
	raise FileNotFoundError('Build directory does not exist.')

bundleroot = (buildroot / 'playground-bundled')
if bundleroot.exists():
	shutil.rmtree(bundleroot)

bundleroot.mkdir()

exe_suffix = '.exe' if os.name == 'nt' else ''
so_prefix = '' if os.name == 'nt' else 'lib'
so_suffix = '.dll' if os.name == 'nt' else '.dylib' if sys.platform.startswith('darwin') else '.so'

playground = pathlib.Path(sys.argv[0]).parent.parent / 'playground'

shutil.copy2(buildroot / 'cacao' / (sys.argv[1] + exe_suffix), bundleroot)
shutil.copytree(playground / 'assets', bundleroot / 'assets')
shutil.copy2(playground / 'launchconfig.cacao.yml', bundleroot)
shutil.copy2(buildroot / 'playground' / ('launch' + so_suffix), bundleroot)
for file in (buildroot / 'playground' / ('launch' + so_suffix + '.p')).rglob('*.spv'):
	shutil.copy2(file, bundleroot / 'assets' / 'shaders')

def get_engine_path():
	return (bundleroot / (sys.argv[1] + exe_suffix))