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

echo "Building GLFW..."
cd ../libs/glfw
rm -rf build
mkdir -p build && cd build
if [ "$(uname)" == "Darwin" ]; then
	cmake .. -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -GNinja -DGLFW_VULKAN_STATIC=ON -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF -DGLFW_BUILD_EXAMPLES=OFF -DBUILD_SHARED_LIBS=ON
	ninja
	cp src/libglfw.dylib ../../generated
elif [ "$(uname)" == "Linux" ]; then
	cmake .. -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -GNinja -DGLFW_VULKAN_STATIC=ON -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF -DGLFW_BUILD_EXAMPLES=OFF -DBUILD_SHARED_LIBS=ON -DGLFW_USE_WAYLAND=ON -DGLFW_USE_X11=ON
	ninja
	cp src/libglfw.so.3.4 ../../generated/libglfw.so
else
	echo "Error: Unknown platform for GLFW!"
	exit 1
fi
cd ../
rm -rf build

echo "Building Glad..."
cd ../glad
clang -fPIC src/gl.c -Iinclude -c -o gl.o
ar -rcs libglad_gl.a gl.o
rm gl.o
mv libglad_gl.a ../generated

echo "Building ImGui backend..."
cd ../imgui
clang++ -fPIC backends/imgui_impl_opengl3.cpp -Ibackends -I. -c -o imguigl3.o
clang++ -fPIC backends/imgui_impl_glfw.cpp -Ibackends -I. -c -o imguiglfw.o
ar -rcs libimgui_gl_glfw.a imguigl3.o imguiglfw.o
rm imguigl3.o imguiglfw.o
mv libimgui_gl_glfw.a ../generated

echo "Building SPIRV-Cross..."
cd ../spirv-cross
rm -rf build
mkdir -p build && cd build
cmake .. -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_POSITION_INDEPENDENT_CODE=ON -GNinja -DSPIRV_CROSS_STATIC=ON -DSPIRV_CROSS_SHARED=OFF -DSPIRV_CROSS_CLI=OFF -DCMAKE_BUILD_TYPE=Debug
ninja
cp ./libspirv-cross*.a ../../generated
cd ../
rm -rf build

cd ../../cacaoglglfw

echo "GLFW + OpenGL backend setup is complete!"