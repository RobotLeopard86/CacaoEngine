# Bundles

```{topic} This page is **up-to-date**! 
The information on this page pertains to the engine post-restructuring.
```

## What are bundles?
Bundles are how you ship your Cacao Engine game. They consist of the Cacao Engine library and runtime, your game binary, the specfile, dependencies, and resources, packed into a specific file structure.  
```{note}
You are not required to use the prebuilt runtime. However, the documentation here will focus on using the prebuilt runtime, which is the recommended one as the engine is designed for it. However, it is possible to build one yourself, though no explicit documentation for this will be created as this is not a principal concern of the Cacao Engine project.
```  
Setting up a basic bundle for just a game binary can be accomplished within the Meson build directory, and an example on how to do this can be found below:  

`meson.build`:
```{code-block}meson
# Game binary
my_game = shared_module('game', sources: [...], dependencies: cacaort_dep, name_prefix: '', install: false)

# Copy runtime files
rtdata = cacao_subproject.get_variable('rtdata')
custom_target('game_runtime', command: [rtdata['setup_script'], rtdata['filelist'], meson.current_build_dir(), '@OUTPUT0@'], output: [host_machine.system() == 'windows' ? 'game.exe' : 'game', rtdata['filenames']], build_by_default: true, depends: rtdata['files'])

# Specfile
spec = configuration_data()
spec.set('BINPATH', fs.relative_to(sandbox, meson.current_build_dir()))
spec.set('ID', 'net.rl86.CacaoSandbox')
spec.set('TITLE', 'Cacao Engine Sandbox Application')
spec.set('VERSION', meson.project_version())
configure_file(input: 'cacaospec.yml.in', output: 'cacaospec.yml', configuration: spec)
```  
`cacaospec.yml.in`:
```{code-block}yaml
meta:
  pkgId: @ID@
  title: @TITLE@
  version: @VERSION@
binary: @BINPATH@
```  

An example for creating a more complex bundle layout will be available at release.

## Bundle Layout (Windows/Linux)
This layout is also used on macOS in build directories, but not in shipping.
* `/` (game directory)
	* `cacao.dll` | `libcacao.so` (Cacao Engine library)
	* `openal.dll` | `libopenal.so.*` (OpenAL-Soft library, engine dependency)
	* `yourgame.exe` | `yourgame` (Cacao Engine runtime, this is the executable you run)
	* `yourgame.dll` | `yourgame.so` (Your game binary, loaded by runtime)
	* `cacaospec.yml` (Cacaospec file)
	* `cacaolicenses.txt` (Cacao Engine license and licenses of engine dependencies)
	* `packs` (asset pack directory)
		* `somepack.xak` (asset pack)
		* other packs
	* `worlds` (world directory)
		* `someworld.xjw` (world)
		* other worlds

## Bundle Layout (macOS, shipped .app bundle)
* `Game.app/Contents/` (app bundle root)
	* `MacOS` (executable directory)
		* `yourgame` (Cacao Engine runtime, this is the executable that gets run)
	* `Frameworks` (dynamic libs directory)
		* `libcacao.dylib` (Cacao Engine library)
		* `libopenal.dylib` (OpenAL-Soft library, engine dependency)
		* `yourgame.dylib` (Your game binary, loaded by runtime)
	* `Resources` (bundle resources directory)
		* `cacaospec.yml` (Cacaospec file)
		* `cacaolicenses.txt` (Cacao Engine license and licenses of engine dependencies)
		* `packs` (asset pack directory)
			* `somepack.xak` (asset pack)
			* other packs
		* `worlds` (world directory)
			* `someworld.xjw` (world)
			* other worlds

## The Cacaospec File
The Cacaospec file defines some basic information about your game and how to launch it. The format for a template is shown above (`cacaospec.yml.in`), but here is the formal schema:
* `meta`: Contains all the metadata about the game
	* `pkgId`: Your game's package ID. This should be in reverse domain format (like Java packages), with the last component in PascalCase (e.g. `com.example.MyProject`)
	* `title`: The user-facing title of your game. Shown in the window titlebar.
	* `version`: The version number of your game build.
* `binary`: The name of your game binary file to load.