#!/usr/bin/env python

import pathlib
import shutil
import sys
import os

rtexe = pathlib.Path(sys.argv[1])
dest = pathlib.Path(sys.argv[2])
destexename = sys.argv[3] + (rtexe.suffix if '.' in rtexe.suffix else '')

if (dest / rtexe.name).exists():
	(dest / rtexe.name).unlink()

if (dest / destexename).exists():
	(dest / destexename).unlink()

shutil.copy2(rtexe, dest / destexename)