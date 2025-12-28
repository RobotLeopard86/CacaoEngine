# Engine Source Code

## Welcome!
Welcome to the engine source code! This is the code that is actually part of the engine and is used for making games.  
This has a few components, so this is a guide to help break them down.  

As a note, the engine is currently undergoing heavy restructuring. Any directories with `OLD` in their names contain code that is now not in use and will eventually be deleted.  

This code will be undergoing active maintennance, updates, and potentially large changes. Do not expect anything in this folder to remain consistent until this notice is removed.  

## Public Headers (`include`)
The public headers are the interface to the engine. They are to be kept commented and clean as described in the [contributing guide](../CONTRIBUTING.md).  

This is the only place in which Cacao Engine headers can be included without the `Cacao/` prefix; that must be used everywhere else.

Ideally, there should be minimal references to implementation details in the public headers, and classes should use the PIMPL paradigm to deal with this. This is for both ABI stability and general cleanliness.  

The only external libraries that may be referenced in the headers are:
* crossguid
* Exathread
* GLM
* Cacao Engine libraries (those in the `libs` directory at the source tree root).

## Engine Core (`src/core`)
The engine core is the platform- and backend-agnostic portion of the engine. Its job is to handle everything that can be achieved in that manner. Most engine responsibilities should be handled here.  

Together with the windowing implementation and graphics backends, it becomes the Cacao Engine shared library that contains the bulk of engine code.

## Linux Windowing (`src/linux`)
Two windowing systems are supported for Linux: X11 and Wayland. Both are enabled by default. Wayland is the more modern successor to X11, and should be prioritized.  

Each of the subdirectories handles the windowing code for that system.

## macOS Windowing (`src/macos`)
This is the windowing integration for macOS using the Cocoa API. This is written in Objective-C++ to allow for integration with Apple's APIs while still being able to operate within the Cacao Engine context.

## OpenGL Backend (`src/opengl`)
The OpenGL backend uses OpenGL 4.1 (no GLES) Core Profile, as described in the backends page in the [manual](https://robotleopard86.github.io/CacaoEngine/dev/manual). It is the only common backend across all three major supported platforms, but may in the future lack more advanced features should they be added, simply due to age.  

This also contains the context management code for each of the supported platforms (with the macOS portion also being written in Objective-C++). EGL is used on both X11 and Wayland.

## Private Headers (`src/private`)
All engine source components have access to these private headers, but they should not be accessible publicly.  

These contain important internal systems; for example the `ImplAccessor`, which grants access to the underlying PIMPL pointers from outside of the class, as well as the implementations of said pointer's classes. For more complete information about the tools found in the private headers, view the [private headers guide](src/private/README.md).

## Game Runtime (`src/runtime`)
The prebuilt game runtime is contained here, and is an executable that handles launching games and loading content from the standard bundle structure. This is the executable that is invoked when a game is launched.  

It is possible to build other runtimes based on the Cacao Engine shared library, but that process is not recommended and undocumented.

## Targetgen Library (`src/shader-tgtgen`)
This is a lightweight library used by the engine core for converting Slang IR into the final shader code to be given to the graphics APIs. It should not use any Cacao Engine APIs.  

The reason is not in `libs` is because it is a tiny component that has no use outside of the engine, but has a distinct enough purpose to be separated from the engine core.

## Vulkan Backend (`src/vulkan`)
The Vulkan backend uses Vulkan 1.3+ with widely-available extensions, as described in the backends page in the [manual](https://robotleopard86.github.io/CacaoEngine/dev/manual). It works on both Windows and Linux and (for now) is the primary backend for implementation and use, except on macOS as only OpenGL is supported there at present.

This also contains the WSI (Windowing System Integration) code for each of the supported platforms, which handles surface creation (enabling the display of content to the window).

## Windows Windowing (`src/win32`)
This is the windowing integration for Windows using the Win32 API. When working in this section, only include `Windows.h` via `Win32Types.hpp`, as that file performs some setup to ensure that the header doesn't cause conflicts.