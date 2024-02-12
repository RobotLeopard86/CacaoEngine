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

Write-Host "Building GLFW..."
cd ../libs/glfw
mkdir -p build && cd build
cmake .. -GNinja -DGLFW_VULKAN_STATIC=ON -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF -DGLFW_BUILD_EXAMPLES=OFF -DBUILD_SHARED_LIBS=ON
ninja
cp src/libglfw.dll ../../generated
cd ../
rm -rf build

Write-Host "Building Glad..."
cd ../glad
zig cc src/gl.c -Iinclude -c -o gl.o
ar -rcs libglad_gl.a gl.o
rm gl.o
mv libglad_gl.a ../generated

Write-Host "Building ImGui backend..."
cd ../imgui
zig c++ backends/imgui_impl_opengl3.cpp -Ibackends -I. -c -o imguigl3.o
zig c++ backends/imgui_impl_glfw.cpp -Ibackends -I. -c -o imguiglfw.o
ar -rcs libimgui_gl_glfw.a imguigl3.o imguiglfw.o
rm imguigl3.o imguiglfw.o
mv libimgui_gl_glfw.a ../generated

cd ../../cacaoglglfw

Write-Host "GLFW + OpenGL backend setup is complete!"