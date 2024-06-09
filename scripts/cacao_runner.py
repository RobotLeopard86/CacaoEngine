#!/usr/bin/env python

import os
import sys
import pathlib
import subprocess

def run(exe_path):
	cacao_exe = pathlib.Path(exe_path).absolute()
	cacao_dir = cacao_exe.parent

	os.chdir(cacao_dir)
	subprocess.run(cacao_exe)
	sys.exit()

if __name__ == '__main__':
	run(sys.argv[1])