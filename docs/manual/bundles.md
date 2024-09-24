# Bundles

## What are bundles?
The entirety of Cacao Engine is contained in the `cacaoengine(.exe)` binary. On its own, the engine will not work correctly. To make it useful, it needs to be part of a bundle. A bundle consists of the engine binary, a launch module, the launch configuration file, and any game assets.

## The Launch Module
The launch module is a compiled binary containing the code of a Cacao Engine game. It must be named `launch.(so|dll|dylib)`, depending on what operating system is being targeted. The launch module has two primary entrypoints or hooks that Cacao Engine calls into to communicate with your game. These are `_CacaoLaunch` and `_CacaoExiting`, which are the startup and shutdown hooks respectively. The [quickstart guide](./quickstart) contains more details about these functions and when they are called.

## The Launch Configuration File
The launch configuration file is a YAML file named `launchconfig.cacao.yml`. It must be placed in the same directory as the Cacao Engine executable, and is loaded by the engine to tell it how to proceed with startup. It contains the following items:  
* `launch`: The path to the launch module relative to the engine executable
* `dynamicTPS`: The number of dynamic ticks that should happen in a second (not a hard constraint)
* `title`: The game window title
* `workingDir`: The working directory that the engine should change to post-launch, relative to the engine executable
* `maxFrameLag`: The number of frames that the engine is allowed to be behind rendering  

## Making a Bundle
Since bundles must be set up in a specific manner, the engine playground as well as the game template both have scripts that automatically generate the bundle. See either repo for the scripts; they are in the `scripts` directory in either repository. If you want to set up a bundle manually, though, here's a typical bundle layout as you might see it on Linux:  
`/`  
 |_ `cacaoengine`  
 |_ `launchconfig.cacao.yml`  
 |_ `launch.so`  
 |_ `assets` (assets folder, not listing what's in there)  

Of course, bundles can follow other layouts. The only two requirements are having the launch configuration file and the engine executable in the same directory, and having the correct path to the launch module directory in the launch configuration file.