import os
import pathlib
import shutil

root = pathlib.Path(os.environ['MESON_SOURCE_ROOT']) / os.environ['MESON_SUBDIR']
patches = root / '_fixes'
tgt = root / 'external'

shutil.copytree(patches, tgt, dirs_exist_ok=True, ignore=lambda _1, _2: 'patch.py')