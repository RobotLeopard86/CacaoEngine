# Cacao Engine Private Headers

In this directory, you can find lots of tools that are used within the engine codebase but should not be exposed to external code. All functionality of the private headers is described below. Nothing from this directory should ever be exported intentionally (`CACAO_API`, `__declspec(dllexport)`, etc.)  

The functionality is grouped by file.  

## `CommandBufferCast.hpp`
This is a small utility function for safely downcasting `unique_ptr` objects that reference generic `CommandBuffer`s to the graphics backend-specific type.  

This is done to allow setup centralization in the base `CommandBuffer` class, while allowing the backend to then obtain its object type to perform non-public actions (such as raw graphics API calls).

## `Freetype.hpp`
This is where the FreeType 2 library instance is stored. It didn't make sense to put it directly inside of the engine class, but there isn't a suitable UI class for it either. Thus, it is stored as a top-level object here and managed by the engine runloop.

## `ImplAccessor.hpp` and the `impl` directory
This is a very powerful utility class system used to access hidden PIMPL objects so that internal engine code can work with those objects without exposing any implementation details to user code.  

The `ImplAccessor` itself is a singleton containing a variety of methods to access PIMPL objects. To make it more convenient to use, the `IMPL` macro is a shorthand for accessing the singleton and calling the appropriate method. For singletons, `IMPL(classname)` is used. For resources, use `IMPL(classname, variablename)`.  

To actually use the returned objects, you will need to include the appropriate header from the `impl` directory, which contains the class definitions of the PIMPL types for supported classes.  

Additionally, there are two helper macros `WIN_IMPL` and `GPU_IMPL` that provide casted access to the appropriate subclass of the `Window` and `GPUManager` PIMPL class respectively. As an example, `GPU_IMPL(Vulkan)` will return a `VulkanGPU` object. The same logic follows for `WIN_IMPL`.  

## `PALConfigurables.hpp`
This header contains template specialization declarations for types supported by `PAL::ConfigureImplPtr`, so that we can restrict what types may be used to call this method.  

The reason this is private is that doing things this way ensures that user code functionally cannot use the `ConfigureImplPtr` method. That method is public because it is for setting up PIMPL objects and used in constructors, so we want to keep things as simple as possible, and using more PIMPL here does not accomplish that.

## `SafeGetenv.hpp`
This is a utility macro that allows for retrieving environment variables as a `std::string` and safely handling errors, because `getenv` is a C function. This wrapper lets us centralize potential failure points and reduce uncertainty as to errors pertaining to `getenv`.

## `SingletonGet.hpp`
This is a tiny helper macro used to declare `Get` functions for the various singleton objects in the engine. It just makes things cleaner and reduces code duplication.