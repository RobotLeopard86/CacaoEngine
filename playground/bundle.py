#!/usr/bin/env python

import pathlib
import shutil
import sys
import os

buildroot = pathlib.Path(sys.argv[1])

if not buildroot.exists():
	raise FileNotFoundError('Build directory does not exist.')

if not (buildroot / 'backends' / sys.argv[2]).exists():
	raise FileNotFoundError('Backend build directory does not exist.')

bundleroot = (buildroot / 'playground.bundled')
if bundleroot.exists():
	shutil.rmtree(bundleroot)

bundleroot.mkdir()

exe_suffix = '.exe' if os.name == 'nt' else ''
so_prefix = '' if os.name == 'nt' else 'lib'
so_suffix = '.dll' if os.name == 'nt' else '.dylib' if sys.platform.startswith('darwin') else '.so'

playground = pathlib.Path(sys.argv[0]).parent

shutil.copy2(buildroot / 'cacao' / ('cacaoengine' + exe_suffix), bundleroot)
shutil.copy2(buildroot / 'backends' / sys.argv[2] / (so_prefix + 'cacaobackend' + so_suffix), bundleroot)
shutil.copytree(playground / 'assets', bundleroot / 'assets')
shutil.copy2(playground / 'launchconfig.cacao.yml', bundleroot)
shutil.copy2(buildroot / 'playground' / ('launch' + so_suffix), bundleroot)
shutil.copy2(buildroot / 'playground' / 'color.vert.spv', bundleroot / 'assets' / 'shaders')
shutil.copy2(buildroot / 'playground' / 'color.frag.spv', bundleroot / 'assets' / 'shaders')

def get_engine_path():
	return (bundleroot / ('cacaoengine' + exe_suffix))