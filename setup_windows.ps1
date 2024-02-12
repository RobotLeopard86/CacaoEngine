Write-Host "Checking for CMake in PATH..."
if (-not(Test-Path "$(which cmake)" -PathType Leaf)) {
	Write-Host "Error: CMake not found in path."
	exit
}
Write-Host "Checking for Ninja in PATH..."
if (-not(Test-Path "$(which ninja)" -PathType Leaf)) {
	Write-Host "Error: Ninja not found in path."
	exit
}
Write-Host "Checking for Zig in PATH..."
if (-not(Test-Path "$(which zig)" -PathType Leaf)) {
	Write-Host "Error: Zig not found in path."
	exit
}

Write-Host "Ensuring submodule initialization..."
git submodule update --init --recursive

Write-Host "Compiling required dependencies..."

rm -rf libs/generated
rm -rf libs/assimp/build
mkdir -p libs/generated

Write-Host "Building Assimp..."
cd libs/assimp
mkdir -p build && cd build
cmake .. -GNinja -DBUILD_SHARED_LIBS=OFF -DASSIMP_BUILD_ZLIB=ON -DASSIMP_NO_EXPORT=ON -DASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT=ON -DASSIMP_INSTALL=OFF -DASSIMP_BUILD_DRACO=ON -DASSIMP_BUILD_SAMPLES=OFF -DASSIMP_BUILD_TESTS=OFF 
ninja
cp lib/libassimp.a lib/libdraco.a ../../generated
cp -R include/assimp ../../generated
cp revision.h draco/draco_features.h contrib/zlib/zconf.h ../../generated

cd ../
rm -rf build

Write-Host "Building stb..."
cd ../stb
zig cc stb_image.c -c -o stb_image.o
ar -rcs libstb_image.a stb_image.o
rm stb_image.o
mv libstb_image.a ../generated
Write-Host "Building ImGui..."
cd ../imgui
$imguisrc="imgui.cpp imgui_tables.cpp imgui_draw.cpp imgui_widgets.cpp imgui_demo.cpp"
foreach ($src in $imguisrc) {
	zig c++ $src -c -o $((Write-Host $src) -replace '.cpp','.o')
}
ar -rcs libimgui.a ./*.o
rm ./*.o
mv libimgui.a ../generated

cd ../..

clear
Write-Host "Cacao Engine setup is complete!"
Write-Host "===================================="
Write-Host
Write-Host "Some components may require additional dependencies to be built."
Write-Host "Check the component directory for a setup_windows.ps1 script like this one."
Write-Host "Be sure to run that before attempting compilation."