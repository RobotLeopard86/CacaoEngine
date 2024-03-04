import os
import shutil

backendLinux = os.path.exists('selected-backend-bin/libcacaobackend.so')
cacaoLibLinux = os.path.exists('cacao/build/libcacao.so')
backendMac = os.path.exists('selected-backend-bin/libcacaobackend.dylib')
cacaoLibMac = os.path.exists('cacao/build/libcacao.dylib')
backendWin = os.path.exists('selected-backend-bin/cacaobackend.dll')
cacaoLibWin = os.path.exists('cacao/build/cacao.dll')
cacaoExeUnix = os.path.exists('cacao/build/cacao')
cacaoExeWin = os.path.exists('cacao/build/cacao.exe')

if not (backendLinux or backendMac or backendWin) or not (cacaoLibLinux or cacaoLibMac or cacaoLibWin) or not (cacaoExeUnix or cacaoExeWin):
	print("One or more of the following does not exist:")
	print("  Selected backend bin symlink")
	print("  Selected backend library")
	print("  Cacao Engine library")
	print("  Cacao Engine loader executable")
	print()
	print("Please verify that all of these exist, then try running this script again.")
	exit(1)

if backendLinux and cacaoLibLinux and cacaoExeUnix:
	shutil.copy2('selected-backend-bin/libcacaobackend.so', 'cacaoplayground')
	shutil.copy2('cacao/build/libcacao.so', 'cacaoplayground')
	shutil.copy2('cacao/build/cacao', 'cacaoplayground')
	print("Updated!")
	exit(0)

if backendMac and cacaoLibMac and cacaoExeUnix:
	shutil.copy2('selected-backend-bin/libcacaobackend.dylib', 'cacaoplayground')
	shutil.copy2('cacao/build/libcacao.dylib', 'cacaoplayground')
	shutil.copy2('cacao/build/cacao', 'cacaoplayground')
	print("Updated!")
	exit(0)

if backendWin and cacaoLibWin and cacaoExeWin:
	shutil.copy2('selected-backend-bin/cacaobackend.dll', 'cacaoplayground')
	shutil.copy2('cacao/build/cacao.dll', 'cacaoplayground')
	shutil.copy2('cacao/build/cacao.exe', 'cacaoplayground')
	print("Updated!")
	exit(0)

print("All files aren't in the same format!")
print("Re-compile them to be in the same format, and run this script again.")
exit(1)