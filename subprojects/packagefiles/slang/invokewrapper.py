import sys
import pathlib
import os

wd = pathlib.Path(sys.argv[0]).parent
cmdline = sys.argv[1:]
exe = pathlib.Path(cmdline[0])
os.chdir(wd)
os.execve(exe, cmdline, os.environ)