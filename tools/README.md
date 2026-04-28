# Cacao Engine Tooling

## Welcome!
Welcome to the engine tooling! This is the suite of command-line tools that may be used to work with Cacao Engine as a game developer.  
Below is a guide to the tools that can be found here.  

## `cubetool`
[View guide](cubetool/README.md)  

`cubetool` is the command-line utility for generating and extracting Cacao Engine cubemap files, which are primarily used for skyboxes. It can either take in a series of images and combine them into a cubemap, or extract face images from an existing cubemap file.

## `matc`
[View guide](matc/README.md)  

`matc` is the command-line utility for compiling Cacao Engine material files into their binary format used at runtime.

## `shaderc`
[View guide](shaderc/README.md)  

`shaderc` is the command-line utility for compiling Slang shader source code into the portable IR format consumed by the engine for final platform code generation at runtime. It achieves a similar result as the default `slangc` compiler tool, but outputs the code in the proper Cacao Engine container format, performs validity checks on the code to ensure it should run in-engine, and configures inclusion of Cacao Engine utilities.

## `worldc`
[View guide](worldc/README.md)  

`worldc` is the command-line utility for compiling Cacao Engine world files into their binary format used at runtime.

## `xak`
[View guide](xak/README.md)  

`xak` is the command-line utility for working with asset packs. It enables creation of a pack from a collection of assets and resources, pack introspection, asset extraction and deletion, and pack merging.