# Cacao Engine  
#### A C++ game engine  

## About
Cacao Engine is a 3D game engine developed by RobotLeopard86. It is written in C++ and uses Meson as the build system.

## Building
See the [build instructions page](https://robotleopard86.github.io/CacaoEngine/latest/manual/building.html) for information.

## Documentation
Documentation is built and deployed automatically to https://robotleopard86.github.io/CacaoEngine.  
If you want to build it yourself, everything is located in the `docs` folder. See the [docs build instructions page](docs/README.md) for more information.

## Branching Scheme
Cacao Engine uses two branches, `main` and `dev`. `main` should be considered stable and should **NEVER** be pushed to on its own. `dev` is where main work can happen and is allowed to be unstable and break. Only once `dev` is in a stable state can it be merged into `main` via a pull request.

## Licensing
Cacao Engine and its auxiliary libraries and tools are licensed under the Apache License 2.0, which can be found in the root directory. All third-party licenses are present in the `licenses` folder.
Any third-party code used by these libraries should have their licenses located within the appropriate project directory (these may not be downloaded until configure-time by Meson).
Additionally, the file `expf.c`, located in the `src` directory, is instead licensed under the [Zero-Clause BSD license](https://opensource.org/license/0bsd). This is because it's a tiny patch that only exists because of an issue with the static Linux math library that sometimes crops up, and this is the closest thing you can get to public domain worldwide.