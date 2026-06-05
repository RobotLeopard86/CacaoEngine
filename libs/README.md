# Cacao Engine Auxiliary Libraries

## Welcome!
Welcome to the auxiliary libraries code! This is the group of useful libraries used by the engine itself and the tooling to work with assets and other data formats.  

Each library is self-contained with no dependency on Cacao Engine's own code; the only dependencies are `libcacaocommon`, a utility header-only library used by all the others, and any external dependencies that each library may require.  

Additionally, all libraries have API documentation available [here](https://robotleopard86.github.io/CacaoEngine/dev/libapis).  

Below is a guide to the libraries that can be found here.

## `libcacaoaudiodecoder`
`libcacaoaudiodecoder` is a simple library that wraps format-specific decoders in a simple interface for decoding an entire audio stream at once in a buffer-to-buffer manner.  

It currently supports MP3, WAV, Ogg Vorbis, and Ogg Opus files.

## `libcacaoimage`
`libcacaoimage` is a library for encoding, decoding, and applying some modifications to images (flipping, channel count adjustment, and bit depth narrowing).  

It currently supports PNG, JPEG, Targa (TGA), TIFF, and WebP files.

## `libcacaoformats`
> [!WARNING]  
> This library is being superseded by the new `libcacaoasset` library, and will be **removed** once that transition is complete.  

`libcacaoformats` is a library for working with Cacao Engine file formats, including asset packs, cubemaps, shaders, materials, and worlds. It deals with both "packed" (binary) and "unpacked" (YAML) files, the former being used at runtime by the engine and the latter being used by developers directly.  

It powers most of the Cacao Engine tooling behind the scenes.

## `libcacaoasset`
`libcacaoasset` is a library for encoding and decoding the new Cacao Engine file formats, which are based on [Jaguar](https://github.com/RobotLeopard86/Jaguar) streams. It succeeds the old `libcacaoformats` library, which dealt with the old, custom formats.  

Unlike its predecessor, `libcacaoasset` works exclusively with the compiled forms of these assets. Direct transformation of human-editable versions into the compiled forms is not a feature provided by this library.