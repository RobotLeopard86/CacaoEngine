# Cacao Engine Shader Compiler

## About
A tool that compiles [Slang](https://shader-slang.org) shader code into Cacao Engine shader objects (`.xjs` files).

## Example
A very simple example shader that conforms to the compiler's rules can be found in this directory as `example_shader.slang`.

## Matrix Layout
The compiler will treat all matrices as being in the column-major layout.

## Command-Line Usage
```
Cacao Engine Shader Compiler 


ce-shaderc [OPTIONS] input...


POSITIONALS:
  input TEXT:FILE ... REQUIRED
                              Input files to compile 

OPTIONS:
  -h,     --help              Print this help message and exit 
  -o TEXT ... Excludes: --auto-output 
                              Compilation output files 
  -A,     --auto-output TEXT Excludes: -o 
                              Automatically generate output files and place them in the 
                              specified directory 
  -q,     --quiet             Suppress all output from the compiler 
  -V,     --verbose           Enable verbose output from the compiler 
  -S,     --spirv Excludes: --glsl 
                              Output in SPIR-V 
  -G,     --glsl Excludes: --spirv 
                              Output in GLSL 
  -v,     --version           Show version info and exit 
```