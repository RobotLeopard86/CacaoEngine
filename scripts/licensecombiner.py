#!/usr/bin/env python3

import sys
import pathlib

# Set up paths
cacao_root = pathlib.Path(sys.argv[0]).parent.parent
cacao_licfile = cacao_root / 'LICENSE'
licenses_dir = cacao_root / 'licenses'

# Scan for licenses
licenses = [
	['Cacao Engine', cacao_licfile]
]
third_party_lics = list(filter(lambda x: x.is_file(), filter(lambda x: not 'tools' in x.parts, licenses_dir.rglob('*'))))
for license in third_party_lics:
	if 'ijg' in license.name:
		continue
	if 'CANARY' in license.name:
		continue
	licenses.append([license.name, license])

# Generate output
with open(sys.argv[1], 'w', encoding='utf8', errors='ignore') as outfile:
	outfile.write("This file contains the licenses of Cacao Engine and all dependency libraries.\n\n")
	for license in licenses:
		outfile.write('=' * 50 + '\n')
		outfile.write('= ' + license[0] + '\n')
		outfile.write('=' * 50 + '\n\n')

		with open(license[1], 'r', encoding='utf8', errors='ignore') as licfile:
			license_text = licfile.read()
			outfile.write(license_text)
			outfile.write('\n' * 2)

		if 'jpeg' in license[0]:
			outfile.write('=' * 35 + '\n')
			outfile.write('= README.ijg (for libjpeg-turbo)\n')
			outfile.write('=' * 35 + '\n\n')
			with open(licenses_dir / 'libcacaoimage' / 'README.ijg', 'r', encoding='utf8', errors='ignore') as ijg:
				ijg_text = ijg.read()
				outfile.write(ijg_text)
				outfile.write('\n' * 2)
