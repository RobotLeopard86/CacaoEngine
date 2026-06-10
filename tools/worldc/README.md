# Cacao Engine World Compiler

## About
A tool that compiles Cacao Engine world definition files (.ajw) to compiled worlds that can be used in a game bundle (.xjw). 

## Command-Line Usage
```
Cacao Engine World Compiler 


ce-worldc [OPTIONS] input...


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
  -v,     --version           Show version info and exit
```