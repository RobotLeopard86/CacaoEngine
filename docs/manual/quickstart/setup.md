# Setup

## Acquire the engine
Currently, Cacao Engine has no precompiled binaries available, as the engine is still in pre-alpha state.

For the time being, you will have to build the engine yourself. Instructions to do that are in the `BUILD.md` document in the GitHub repository and [here](../building).

Before you get continue, make sure you have installed all the dependencies listed there.

## Bundle layout
Cacao Engine itself ships as an executable. Your code lives in a special file called the launch module. This launch module is a file that must be named `launch.(dll,so,dylib)`. You can build your launch module through whatever means you like, though [Meson](https://mesonbuild.com) is the only officially supported option. In addition to your launch module, you will need another file: `launchconfig.cacao.yml`. This is a YAML file that must be placed in the same dirctory as the built Cacao Engine executable, which has the following items:
* `launch`: The path to the launch module relative to the engine executable
* `dynamicTPS`: The number of dynamic ticks that should happen in a second (**not a hard constraint**)
* `title`: The game window title
* `workingDir`: The working directory that the engine should change to post-launch, relative to the engine executable
* `maxFrameLag`: The number of frames that the engine is allowed to be behind rendering

A typical bundle directory might look something like this:  
`/`  
 |_ `cacaoengine(.exe)`  
 |_ `launchconfig.cacao.yml`  
 |_ `launch.(so|dll|dylib)`  
 |_ `assets`  
