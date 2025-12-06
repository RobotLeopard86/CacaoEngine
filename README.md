# Cacao Engine  
#### A C++ game engine  

## About
Cacao Engine is a general-purpose 3D game engine, written in C++ and using Meson as the build system.  

## Building
See the [build instructions page](https://robotleopard86.github.io/CacaoEngine/dev/manual/building.html) for detailed instructions.

## Project Management
Cacao Engine project management happens on [Taiga](https://tree.taiga.io/project/robotleopard86-cacao-engine). Issues created in GitHub will be automatically synced to Taiga for tracking purposes.  
Please **do not create issues in Taiga**, as they will not be synced to GitHub (a limitation of Taiga's GitHub integration).

## Contributing
If you wish to contribute to Cacao Engine, please consult the [contribution guidelines page](CONTRIBUTING.md) for more information.

## Documentation
Documentation is built and deployed automatically to https://robotleopard86.github.io/CacaoEngine.  
If you want to build it yourself, everything is located in the `docs` folder. See the [docs build instructions page](docs/README.md) for more information.

## Licensing
Cacao Engine and its auxiliary libraries and tools are licensed under the Apache License 2.0, which can be found in the root directory. All third-party licenses are present in the `licenses` directory, grouped by which component uses them. Any library used directly by multiple components will have its license present at the top level of the `licenses` directory.  

Transitive dependency libraries that are compiled into engine binaries and used at runtime will have their respective licenses located in the `transitive` directory within the `licenses` directory.  
All other transitive dependencies that are not involved in the build process or used at runtime will have their respective licenses found in the subproject directory of the owning project. This will not be downloaded until configure-time by Meson.

Additionally, the file `expf.c`, located in the directory `engine/src/core/src`, is instead licensed under the [Zero-Clause BSD license](https://opensource.org/license/0bsd). This is because it's a tiny patch that only exists because of an issue with the static Linux math library that sometimes crops up, and this is the closest thing you can get to public domain worldwide.  

All Git patches and Meson build definitions located in `subprojects/packagefiles` are provided under the [MIT License](https://opensource.org/license/mit) instead, as this is the same license used by Meson wraps.
