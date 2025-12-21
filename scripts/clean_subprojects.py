#!/usr/bin/env python3
import sys
import pathlib
import glob
import os
import shutil
import stat

# Directories
basedir = pathlib.Path(sys.argv[0]).parent.parent
subdir = basedir / 'subprojects'

# Read gitignore
with open(basedir / '.gitignore') as gitignore:
	global ignore
	ignore = gitignore.readlines()

# Get a list of expressions to exclude
sublines = list(filter(lambda x: 'subprojects' in x, ignore))
excludes = list(map(lambda y: y[1:][:-1], filter(lambda x: x[0] == '!', sublines)))
deletewraps = list(map(lambda y: pathlib.PurePath(y[:-1]).name, filter(lambda x: '.wrap' in x and x[0] != '!', sublines)))

# Expand globs
globbed = list(map(lambda x: glob.glob(pathlib.PurePath(x).name, root_dir=subdir, include_hidden=True), excludes))
flattened = [
	x
	for y in globbed
	for x in y
]
keep = list(filter(lambda x: not x in deletewraps, flattened))

# Decide what to remove
subfiles = os.listdir(subdir)
remove = list(set(subfiles).difference(set(keep)))

# Remove
for r in remove:
	p = subdir / r
	print('Removing', p)
	if p.is_dir():
		for p2 in p.rglob('*'):
			if p2.is_file():
				p2.chmod(p.stat().st_mode | stat.S_IWRITE)
		shutil.rmtree(p)
	else:
		os.remove(p)

print('Complete.')