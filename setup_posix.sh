#!/bin/sh

echo "Checking for CMake in PATH..."
if ! [ -f $(which cmake) ]; then
	echo "Error: CMake not found in path."
	exit 1
fi
echo "Checking for Ninja in PATH..."
if ! [ -f $(which ninja) ]; then
	echo "Error: Ninja not found in path."
	exit 1
fi
echo "Checking for Zig in PATH..."
if ! [ -f $(which zig) ]; then
	echo "Error: Zig not found in path."
	exit 1
fi

echo "Ensuring submodule initialization..."
git submodule update --init --recursive

echo "Compiling required dependencies..."

rm -rf libs/generated
rm -rf libs/assimp/build
mkdir -p libs/generated

echo "Building Assimp..."
cd libs/assimp
mkdir -p build && cd build
cmake .. -GNinja -DBUILD_SHARED_LIBS=OFF -DASSIMP_BUILD_ZLIB=ON -DASSIMP_NO_EXPORT=ON -DASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT=ON -DASSIMP_INSTALL=OFF -DASSIMP_BUILD_DRACO=ON -DASSIMP_BUILD_SAMPLES=OFF -DASSIMP_BUILD_TESTS=OFF 
ninja
cp lib/libassimp.a lib/libdraco.a ../../generated
cp -R include/assimp ../../generated
cp revision.h draco/draco_features.h contrib/zlib/zconf.h ../../generated

cd ../
rm -rf build

echo "Building stb..."
cd ../stb
zig cc stb_image.c -c -o stb_image.o
ar -rcs libstb_image.a stb_image.o
rm stb_image.o
mv libstb_image.a ../generated
echo "Building ImGui..."
cd ../imgui
imguisrc="imgui.cpp imgui_tables.cpp imgui_draw.cpp imgui_widgets.cpp imgui_demo.cpp"
for src in $imguisrc; do
	zig c++ $src -c -o $(echo $src | sed 's/\.cpp/\.o/g')
done
ar -rcs libimgui.a ./*.o
rm ./*.o
mv libimgui.a ../generated

cd ../..

clear
echo "Cacao Engine setup is complete!"
echo "===================================="
echo
echo "Some components may require additional dependencies to be built."
echo "Check the component directory for a setup_posix.sh script like this one."
echo "Be sure to run that before attempting compilation."