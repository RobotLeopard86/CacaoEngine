# Cacao Engine  
#### A C++ game engine  

## About
Cacao Engine is a game engine developed by RobotLeopard86. It is written in C++ and uses Meson as the build system.

## Planned Features
* 2D and 3D rendering capabilties
* Level editor
* C++, C#, and Python scripting
* Support for Windows, macOS, and GNU/Linux
* AI engine
* Physics system
* Support for multiple windowing systems and rendering APIs
* Controller support

More features may come in the future, so stay tuned!

## Building
See the [build instructions page](BUILD.md) for information.

## Branching Scheme
Cacao Engine uses two branches, `main` and `dev`. `main` should be considered stable and should **NEVER** be pushed to on its own. `dev` is where main work can happen and is allowed to be unstable and break. Only once `dev` is in a stable state can it be merged into `main` via a pull request.

## Licensing
Cacao Engine is licensed under the Apache License 2.0, which can be found in the root directory. All third-party licenses are present in the `licenses` folder.
Any third-party code used by these libraries should have their licenses located within the appropriate project directory (these may not be downloaded until build-time by Meson).
Additionally, the file `expf.c`, located in the `cacao` directory, is designated entirely to the public domain to the extent applicable by law. This is because it's a stupid tiny patch that only exists because of weirdness with the Linux math library that I have encountered, and frankly don't care about.