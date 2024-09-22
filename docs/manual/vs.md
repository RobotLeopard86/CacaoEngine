# Visual Studio Compatibility

**IMPORTANT**: Only Visual Studio 2022 is supported!

## Workloads
Cacao Engine uses the following Visual Studio workloads:
* Desktop development with C++
* Game development with C++

## Why not to use Meson's Visual Studio generator
Cacao Engine makes use of Meson run targets. However, in the Visual Studio generator, Meson turns these run targets into Visual Studio build targets.  
Since Cacao Engine will refuse to start up when not bundled correctly, a run target is used to bundle the engine and also to run the playground.  
Since the engine is running during what Visual Studio thinks is building the engine, the debugger is not accessible and things just generally break.

## What to do instead
1. Run your setup command twice. The first time, name the build directory `Debug`.  The second time, name it `Release`, and also add `--buildtype=release` to your Meson command line.  
2. Run the Python script `vsgen.py` from the project root directory. This will generate Visual Studio project and solution files.