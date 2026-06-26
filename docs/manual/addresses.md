# Resource Addresses

Loading anything related to your game in Cacao Engine requires you to know its address. This is a specially-formatted string describing how to fetch the resource and its identifier.  Specifically, it is formated like this: `<type prefix>:<identifier>`. Note that all addresses are case-sensitive. Unless explicitly specified by the resource type (such as a `/` for a separator), only alphanumeric characters and underscores are permitted in the identifier.   

As a general note, the engine will refuse to load unpacked Cacao Engine formats at runtime. For that reason, referencing an unpacked-format file will fail as the engine won't be able to discover it.

Below, the different formats are specified.

## Blob Resources
Blob resources are stored in asset packs and are blobs of data that the engine does not interfere with. They can contain any content, whether that be JSON, some binary data, or whatever else.  

Their addresses use the type prefix `r`, and the identifier is the path to the resource within the asset pack. This mirrors the structure of the resource folder used to create the pack. It uses forward slashes for directory deliminators, but does not support using `..` or anything like that.

## Assets
Assets are contained in an asset pack and are resources that have a direct engine-related use. At present, this is:
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

## Models
Models are stored in asset packs, and may themselves be loaded by directly using their address. However, often, you want to refer to the contained assets (namely meshes and textures). Thus, a special address format exists for referencing these contained assets. You cannot use need these special addresses when working with the `Model` class API, but the resource manager will accept them and cache the model for future requests.

### Meshes
Meshes, unlike most assets, are not contained directly within asset packs, but instead within models. To access a mesh, use its special address format, which uses the type prefix `m`. The identifier consists of the containing model's identifier, a `/`, then the mesh's name within the model.

### Textures
Though many textures are stored externally to a model file, some formats contain embedded textures. To access these embedded textures, use their special address format, which has the type prefix `e`. The identifier consists of the containing model's identifier, a `/`, then the texture's name within the model.

## Worlds
Worlds are where your game takes place. They are stored separately from assets, but may reference them. Loading a world will often trigger asset loading as well. World discovery will fail if the world is not in the proper location.  

Their addresses use the type prefix `w`, and the identifier is the name of the world.