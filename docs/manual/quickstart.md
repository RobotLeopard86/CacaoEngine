# Quickstart

```{warning}
This guide is still a work-in-progress!
```

So, you want to get started with Cacao Engine? This guide will teach you how to do so, as well as give you a basic introduction to the systems that you'll use to build a game. Please note that as precompiled binaries for Cacao Engine are not yet available, this procedure will have you build the engine from source alongside your game. This will change in the future.

## 1. Environment Setup
The first thing you need to do is setup your build machine to be able to compile Cacao Engine. To do this, follow the [build instructions](./building) through to the "Compiler Support" section. Then, come back here.  

## 2. Creating the Skeleton Project
Next, you'll need to configure your build. To do this, you need to copy the files below into your project. The names are very important. You don't need to worry too much about what all of this does, even though it's a lot. We'll come back to it later.

`native.ini`*:  
```ini
[binaries]
c = 'clang'
cpp = 'clang++'
c_ld = 'lld'
cpp_ld = 'lld'
objc = 'clang'
objcpp = 'clang++'
objc_ld = 'lld'
objcpp_ld = 'lld'

[cmake]
CMAKE_C_COMPILER = 'clang'
CMAKE_CXX_COMPILER = 'clang++'
LINKER_TYPE = 'lld'
```  
```{note}
While you can opt to build with something other than Clang and LLD, which would mean excluding this file or modifying its fields, we highly discourage this, even on Windows. This is the primary compiler and linker configuration used for Cacao Engine on all platforms and is the principal one supported.
```  

`meson.build`:
```meson
project('your_project', 'c', 'cpp', meson_version: '>=1.10.2', default_options: [ 'cpp_std=c++20', 'b_vscrt=from_buildtype', 'default_library=static' ])

# Cacao Engine
cacao = subproject('cacaoengine', required: true, default_options: {
	'backends': ['opengl', 'vulkan'],
	'build_sandbox': false
})

# Game binary
your_project = shared_module('your_project', sources: [
    'src' / 'main.cpp',
	cacao.get_variable('engine_reflection')
], include_directories: 'src', dependencies: cacao.get_variable('cacaort_dep'), name_prefix: '', install: false)

# Copy runtime executable
rtdata = cacao.get_variable('rtdata')
custom_target('rtsetup', command: [rtdata['setup_script'], rtdata['filelist'], meson.current_build_dir(), '@OUTPUT0@'], output: [host_machine.system() == 'windows' ? 'your_project.exe' : 'your_project', rtdata['filenames']], build_by_default: true, depends: rtdata['files'])

# Specfile
spec = configuration_data()
spec.set('BINPATH', import('fs').relative_to(your_project, meson.current_build_dir()))
spec.set('ID', 'com.example.YourProject')
spec.set('TITLE', 'Your Project Title')
spec.set('VERSION', meson.project_version())
configure_file(input: 'cacaospec.yml.in', output: 'cacaospec.yml', configuration: spec)

# Resource configuration
subdir('packs')
subdir('worlds')
```  

`subprojects/cacaoengine.wrap`:
```ini
[wrap-git]
url = https://github.com/RobotLeopard86/CacaoEngine
depth = 1
revision = dev
```

`src/main.cpp`:
```cpp
#include "cacaoengine.astra.hpp"

#ifdef _MSC_VER
#define CALL __cdecl
#else
#define CALL
#endif

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

extern "C" EXPORT void CALL __CacaoRegisterTypes() {}
```  

`packs/meson.build`:
```
# Nothing here for now, but you do need to create this file!
```

`worlds/meson.build`:
```meson
custom_target('yourfirstworld', output: 'yourfirstworld.xjw', input: ['yourfirstworld.ajw'],
	console: true, command: [cacao.get_variable('worldc'), '-V', '@INPUT@', '-o', '@OUTPUT@'], build_by_default: true)
```  

`worlds/yourfirstworld.ajw`:
```yaml
cam:
  position:
    x: 0
    y: 0
    z: 0
  rotation:
    x: 0
    y: 0
    z: 0
    w: 1
  clear: FF00FF
  orthographic: false
actors: []
```  

`cacaospec.yml.in`:
```yaml
meta:
  pkgId: @ID@
  title: @TITLE@
  version: @VERSION@
binary: @BINPATH@
startupWorld: w:yourfirstworld
```

## 3. The First Build
Okay, your skeleton project is now ready to go. Now it's time to build it for the first time. You'll need your terminal for this part; open up your project directory in it and run this command: `meson setup build --native-file native.ini`. This will configure your build directory, but be warned: it may take a while as it needs to download all the dependencies.  

Once that completes, run `meson compile -C build` to compile your code and build your project. This **will** take a while. If everything goes well, you should have your project ready to go. Run the executable at `build/your_project` or `build\your_project.exe` (on Windows), and you should see, after a bit, a blank purple window. If you see that, congratulations, Cacao Engine is working!

TODO