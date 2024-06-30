import subprocess
import re
import sys

def dump_symbols(lib_path, def_path):
	# Use dumpbin to get the symbols
	result = subprocess.run(['dumpbin', '/linkermember:1', lib_path], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

	if result.returncode != 0:
		exit(1)

	# Parse the dumpbin output to extract symbols
	symbols = []
	dump = False
	for line in result.stdout.splitlines():
		if " public symbols" in line:
			dump = True
			continue
		
		if "Summary" in line:
			break

		if dump and line.strip() != '':
			symbol = line.split()[-1]
			# Exclude symbols that start with certain prefixes or match unwanted patterns
			if not re.match(r'^(\@\_|__real)', symbol):
				symbols.append(symbol)

	# Write the symbols to the .def file
	with open(def_path, 'w') as def_file:
		def_file.write("EXPORTS\n")
		for symbol in symbols:
			def_file.write(f"\t{symbol}\n")
	

if __name__ == "__main__":
	if len(sys.argv) != 3:
		print("Usage: python", sys.argv[0], "<path_to_lib> <path_to_def>")
		exit(1)
	else:
		dump_symbols(sys.argv[1], sys.argv[2])
	exit(0)
