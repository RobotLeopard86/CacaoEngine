#!/usr/bin/env python3

import pathlib
import shutil
import sys

# Get the list of required runtime files
rtfiles = list(map(lambda p: pathlib.Path(p), pathlib.Path(sys.argv[1]).read_text().splitlines()))

# The runtime executable always has to be the first file listed
rtexe = rtfiles[0]

# Get the destination data
dest = pathlib.Path(sys.argv[2])
destexename = pathlib.PurePath(sys.argv[3]).name

# Remove the destination executable if it exists
if (dest / destexename).exists():
	(dest / destexename).unlink()

# Copy the runtime executable to the directory
shutil.copy2(rtexe, dest / destexename)
rtfiles.remove(rtexe)

# Copy all the other files
for file in rtfiles:
	if (dest / file.name).exists():
		(dest / file.name).unlink()
	shutil.copy2(file, dest / file.name)
