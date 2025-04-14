# Cacao Engine Cubemap Tool

## About
A tool to create packed cubemap (.xjc) files from unpacked files (.ajc) and their associated images, as well as extract face images from existing packed cubemaps.

## Supported Formats
Input images referenced in an unpacked cubemap file may be in one of these formats:
* JPEG
* PNG
* TGA
* BMP
* HDR

Output images will always be stored as PNG files, as that's how the packed format stores them.

## Command-Line Usage
```
Cacao Engine Cubemap Tool 


ce-cubetool [OPTIONS] SUBCOMMAND


OPTIONS:
  -h,     --help              Print this help message and exit 
  -v,     --version           Show version info and exit 
  -q,     --quiet             Suppress all output from the compiler 
  -V,     --verbose           Enable verbose output from the compiler 

SUBCOMMANDS:
  create                      Create a new cubemap 
  extract                     Extract face images from a cubemap
```
```
Create a new cubemap 


ce-cubetool create [OPTIONS] input


POSITIONALS:
  input TEXT:FILE REQUIRED    Path to an unpacked cubemap definition file (.ajc) to read as 
                              input 

OPTIONS:
  -h,     --help              Print this help message and exit 
  -o TEXT REQUIRED            Output file path
```
```
Extract face images from a cubemap 


ce-cubetool extract [OPTIONS] input


POSITIONALS:
  input TEXT:FILE REQUIRED    Path to an packed cubemap file (.xjc) to read as input 

OPTIONS:
  -h,     --help              Print this help message and exit 
  -A,     --all-faces Excludes: --face 
                              Extract all face images from the cubemap 
  -f,     --face TEXT ... Excludes: --all-faces 
                              Extract specific faces from the cubemap (left, right, up, down, 
                              front, back) 
  -o TEXT                     Directory to place output files in
```