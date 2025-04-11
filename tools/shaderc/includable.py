import os
import sys
import pathlib

outdir = pathlib.Path(os.environ['MESON_BUILD_ROOT']) / os.environ['MESON_SUBDIR']
outfn = pathlib.Path(sys.argv[1]).name.removesuffix('.slang') + '.inc'
outfile = outdir / outfn

contents = ''
with open(sys.argv[1], 'r') as f:
    contents = f.read()

with open(outfile, 'w') as f:
    f.seek(0)
    f.truncate()
    f.writelines([
        'R"(',
        contents,
        ')";'
    ])