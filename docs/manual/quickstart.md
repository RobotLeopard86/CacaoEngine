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
Okay, your skeleton project is now ready to go. Now it's time to build it for the first time. You'll need your terminal for this part; open up your project directory in it and run the appropriate command below:
* Windows: `meson setup build --vsenv --native-file native.ini`  
* macOS/Linux: `meson setup build --native-file native.ini`  
This will configure your build directory, but be warned: it may take a while as it needs to download all the dependencies.  

Once that completes, run `meson compile -C build` to compile your code and build your project. This **will** take a while. If everything goes well, you should have your project ready to go. Run the executable at `build/your_project` or `build\your_project.exe` (on Windows), and you should see, after a bit, a blank purple window. If you see that, congratulations, Cacao Engine is working!

## 4. Drawing Something
Congratulations, you now have a bootstrapped Cacao Engine game. It's a little boring, though; a purple screen isn't that interesting. Now, we'll add a cube. To start, we're going to need a material.  

First, replace the contents of `packs/meson.build` with this:
```meson
# Pack a0
a0_pak = custom_target('a0', output: 'a0.xak', input: [cube_mat], console: true, command: [cacao.get_variable('xak'), '-V', 'create', '-a', meson.current_build_dir() / '..' / 'assets', '-o', '@OUTPUT@'], build_by_default: true)
```  

Then, make a new directory in the project called `assets`, and copy this into `assets/meson.build`:  
```meson
cube_mat = custom_target('cube.xjm', output: 'cube.xjm', input: 'cube.ajm', console: true, build_by_default: true, command: [cacao.get_variable('matc'), '-V', '@INPUT@', '-o', '@OUTPUT@'])
```

Now, we just need to write the material for the cube into `assets/cube.ajm`; it should look like this:
```yaml
shaderAddress: a:builtin_basic_color_shader
renderMode: opaque
parameters:
  - target: color
    value: [1.0, 1.0, 1.0, 1.0]
```

And finally, to hook it all up, add `subdir('assets')` to the root `meson.build`, right above the `subdir('packs')` line. Rebuild the project, run it again, and... you should see that nothing has changed. We haven't actually added a cube yet! Let's do that now. Go into `worlds/yourfirstworld.ajw` and find this line:
```yaml
actors: []
```  

Replace that with this:
```yaml
actors:
  - name: Cube
    active: true
    guid: aab4d03b-1f4e-4829-bcc2-531e41169d19
    parent: 0
    transform:
      position:
        x: 0
        y: 0
        z: 5
      rotation:
        x: 0
        y: 0
        z: 0
        w: 1
      scale:
        x: 1
        y: 1
        z: 1
    components:
      - id: cacao.meshRenderer
        enabled: true
        rfl:
          mesh: a:builtin_cube
          material: a:cube0
```  

Now, build and run the program again. You should now see a white square on screen.

## 5. Setting Up a Basic Script
A white square is cool, but what if we want to see some interactivity? For that, we'll need a script. Create a new file in the `src` folder called `spinner.hpp`, and copy this into it:  
```cpp
#pragma once

#include "Cacao/Script.hpp"

using namespace Cacao;

class ASTRA_REFLECT Spinner : public Script {
  public:
    Spinner() {}

    void OnDynTick(float timestep) override;

    ASTRASETUP(Spinner);
};
```  

You may be wondering what some of those macros are. These are what allow Cacao Engine's reflection generator to generate the information about your class that's needed in order for the engine to be able to instantiate your component from the world file. Next, you'll need to add the reflection generator to your build process. Add this to your root `meson.build`, right after the Cacao Engine subproject but before the game binary:  
```meson
# Reflection
reflection = custom_target('yourproject.astra.hpp', output: 'yourproject.astra.hpp',
	input: ['src' / 'spinner.hpp'], console: true, command: [cacao.get_variable('astra').get_variable('astra_generator'), '-c', meson.global_build_root(), '-o', 
	meson.current_build_dir(), '-n', 'your_project', '-s', cacao.get_variable('engine_reflection')[0].full_path().replace('hpp', 'json'), '@INPUT@'], depends: cacao.get_variable('engine_reflection'))
```  

And be sure to add the `reflection` variable to your game binary sources list.  

Now, we need to implement the component in code, so go into `main.cpp`, and paste this to replace the `extern "C"` line:
```cpp
#include "spinner.hpp"
#include "yourproject.astra.hpp"

#include "glm/trigonometric.hpp"

#include "Cacao/CodeRegistry.hpp"

void Spinner::OnDynTick(float timestep) {
  constexpr static float degPerSec = -30;
  Transform transform = GetOwner()->GetLocalTransform();
  glm::quat rotDelta = glm::angleAxis(glm::radians(degPerSec * timestep), glm::vec3 {0, 1, 0});
  transform.SetRotation(rotDelta * transform.GetRotation());
  GetOwner()->SetLocalTransform(transform);
}

extern "C" EXPORT void CALL __CacaoRegisterTypes() {
	CodeRegistry::Get().RegisterFactory<Component>("spinner", []() { return new Spinner(); }, typeid(Spinner));
}
```  

And finally, we need to attach the component to the cube. So, go back into `worlds/yourfirstworld.ajw`, and add this:  
```yaml
      - id: spinner
        enabled: true
        rfl: {}
```  
Note that the indentation is important; this is intentionally indented to line up with where it needs to be for correct YAML syntax in the world file.  

Build and run the program again, and the white cube should be spinning!  

This concludes the quickstart guide; for further details, please consult the rest of the documentation.