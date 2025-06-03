# Resource Addresses

```{topic} This page is **up-to-date**! 
The information on this page pertains to the engine post-restructuring.
```

Loading anything related to your game in Cacao Engine requires you to know its address. This is a specially-formatted string describing how to fetch the resource and its identifier.  Specifically, it is formated like this: `<type prefix>:<identifier>`. Note that all addresses are case-sensitive.  

As a general note, the engine will refuse to load unpacked Cacao Engine formats at runtime. They will only be loaded in the editor. For that reason, referencing an unpacked-format file will fail as the engine won't be able to discover it.

Below, the different formats are specified.

## Resources
Resources are the base type of anything that can be loaded. This section is about **blob resources** (often just called resources), which are stored in asset packs and are blobs of data that the engine does not interfere with. They can be literally anything.  

Their addresses use the type prefix `r`, and the identifier is the path to the resource within the asset pack. This mirrors the structure of the resource folder used to create the pack. It uses forward slashes for directory deliminators, but does not support using `..` or anything like that.

## Assets
Assets are anything that is contained in an asset pack and is of a certain type deemed useful by the engine. At present, this is:
* 2D Textures
* Cubemaps
* Sounds
* Models
* Fonts
* Shaders
* Materials

Their addresses use the type prefix `a`, and the identifier is the asset address as specified in the asset pack.  
```{tip}
If you use auto-generated asset addresses for your packs and don't recall the asset address, use the `ce-xak` tool to list the assets in a pack.
```

## Worlds
Worlds are where your game takes place. They are stored separately from assets, but may reference them. Loading a world will often trigger asset loading as well. World discovery will fail if the world is not in the proper location.  

Their addresses use the type prefix `w`, and the identifier is the name of the world.

## Binary Objects
Binary objects refer to objects accessible in game binaries (components and services). Due to the sensitive and interconnected nature of code, binary objects **do not** use the overlay stack and are thus not possible to override.

Their addresses use the type prefix `b` and the identifier follows the format `<svc|com>@<package>/<object identifier>`. Use `svc` for services and `com` for components.