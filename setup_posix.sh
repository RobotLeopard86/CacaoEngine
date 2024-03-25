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
echo "Checking for Clang in PATH..."
if ! [ -f $(which clang) ]; then
	echo "Error: Clang not found in path."
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
cmake .. -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -GNinja -DBUILD_SHARED_LIBS=ON -DASSIMP_BUILD_ZLIB=ON -DASSIMP_NO_EXPORT=ON -DASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT=ON -DASSIMP_INSTALL=OFF -DASSIMP_BUILD_SAMPLES=OFF -DASSIMP_BUILD_TESTS=OFF
ninja
cp bin/libassimp.so.5.2.5 ../../generated/libassimp.so
cp revision.h contrib/zlib/zconf.h ../../generated
cp -R include/assimp ../../generated

cd ../
rm -rf build

echo "Building yaml-cpp..."
cd ../yaml-cpp
mkdir -p build && cd build
cmake .. -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -GNinja -DBUILD_SHARED_LIBS=OFF -DYAML_BUILD_TOOLS=OFF -DYAML_CPP_INSTALL=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=ON
ninja
cp libyaml-cpp.a ../../generated

cd ../
rm -rf build

echo "Building stb..."
cd ../stb
clang -fPIC stb.c -c -o stb.o
ar -rcs libstb_image.a stb.o
rm *.o
mv libstb_image.a ../generated

echo "Building ImGui..."
cd ../imgui
imguisrc="imgui.cpp imgui_tables.cpp imgui_draw.cpp imgui_widgets.cpp imgui_demo.cpp"
for src in $imguisrc; do
	clang++ -fPIC $src -c -o $(echo $src | sed 's/\.cpp/\.o/g')
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