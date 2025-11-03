import sys
import os
import pathlib
import re

sourcedir = pathlib.Path(sys.argv[1])
filter = re.compile(".*" if len(sys.argv) < 3 else sys.argv[2])
root = pathlib.Path(os.environ['MESON_SOURCE_ROOT']) / os.environ['MESON_SUBDIR']

for file in sourcedir.iterdir():
    if re.match(filter, file.name) != None and (file.suffix == '.cpp' or file.suffix == '.h' or (file.suffix == '.mm' and os.name == 'darwin')):
        print(file.relative_to(root))
    if file.is_dir():
        if os.name == 'nt' and file.name == 'windows':
            for f2 in file.iterdir():
                if re.match(filter, file.name) != None and (f2.suffix == '.cpp' or f2.suffix == '.h'):
                    print(f2.relative_to(root))
        elif file.name == 'unix':
            for f2 in file.iterdir():
                if re.match(filter, file.name) != None and (f2.suffix == '.cpp' or f2.suffix == '.h' or (f2.suffix == '.mm' and os.name == 'darwin')):
                    print(f2.relative_to(root))