import sys
import pathlib

print("\033[93;1mWARNING:\033[0m You are using a compiler configuration that is UNSUPPORTED by Cacao Engine.\nThis build may not compile, link, or function correctly, and no support will be offered.\n\nAre you sure that you want to continue?\n")
response = input("Type '\033[1myes\033[0m' to proceed: ").lower()
if response == 'yes':
	okfile = pathlib.Path(sys.argv[1])
	try:
		with open(okfile, 'x') as ok:
			ok.write('ok')
	except FileExistsError:
		pass
	raise SystemExit(0)
else:
	raise SystemExit(1)
