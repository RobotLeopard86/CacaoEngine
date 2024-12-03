# HLSL Shaders

**WARNING**: The `dxc` version that ships with Visual Studio 2022 **does not** support SPIR-V codegen!! You will need to either use the `dxc` from the [Vulkan SDK](https://vulkan.lunarg.org) or build it from source.  

Cacao Engine shaders should target Shader Model 6.0.  

## Matrix Packing
Shaders *MUST* use row-major matrix packing because the DirectX Shader Compiler changes that to column-major in SPIR-V, which is what Cacao Engine uses. See [this section](https://github.com/microsoft/DirectXShaderCompiler/blob/main/docs/SPIR-V.rst#appendix-a-matrix-representation) of the SPIR-V compatibility document for more info on why. Take a look at the whole of that document too; it will give better context as to writing HLSL for SPIR-V.  
You can either mark each matrix with the `row_major` qualifier, or add `#pragma pack_matrix(row_major)` to the top of your file.

## Compile Arguments
When using `dxc` for shader compilation, you **MUST** ensure that the `-fvk-use-gl-layout` flag is added to the command line, or else the engine may not be able to use your shader.

## Main Functions
The main functions of all shader stages must have the name `main`, otherwise Cacao Engine will fail to use them properly.  

## Inputs
All vertex shader inputs should be within a `VSInput` struct. The struct can contain the following members:
| Data Type | Semantic |
| --------- | -------- |
| `float3` | `POSITION0` |
| `float2` | `TEXCOORD0` |
| `float3` | `TANGENT0` |
| `float3` | `BITANGENT0` |
| `float3` | `NORMAL0` |  

## Constant Buffers
Cacao Engine sends engine data to shaders via a constant buffers. Below is an example of how this should be declared. **The order of members  is important!**  
```{code-block} hlsl
struct CacaoGlobals {
  float4x4 projection;
  float4x4 view;
};
ConstantBuffer<CacaoGlobals> globals : register(b0);
```  

## Local Object Data
All data pertaining to an individual object (the transformation matrix plus any custom shader data) must be part of a struct named `ObjectData`, and must have an instance declared that is marked with the `[[vk::push_constant]]` annotation and named `object`. Here's an example:  
```{code-block} hlsl
struct ObjectData {
  float4x4 transform;
  //Custom shader data goes here...
};

[[vk::push_constant]] ObjectData object;
```  

## Applying the Matrices
Since all of the matrices are row-major, apply them in the order below. This example assumes the output struct contains a member declared as `float4 Pos : SV_POSITION;`, the output struct is named `output`, the input struct contains a member declared as `float3 Pos : POSITION0;` and that the input struct is named `input`.  
```{code-block} hlsl
output.Pos = mul(input.pos, mul(object.transform, mul(globals.view, globals.projection)));
```  

## Textures and Samplers
In HLSL, samplers and textures are separate objects. This model is not compatible with all APIs. A `SamplerState` associated with a texture must have the same register number as that texture (e.g. `register(t0)` on the texture is connected to the sampler with `register(s0)`). `SamplerState`s and textures must also all be marked with the `[[vk::combinedImageSampler]]` annotation. In addition, you all textures must be given a binding number. This is different than HLSL registers, and they are shared between the vertex and fragment stages. Since binding `0` and `1` is used by the engine constant buffer, you must start with binding number `1`. Apply bindings with the `[[vk::binding(NUMBER)]]` annotation. Here's an example:  
```{code-block} hlsl
[[vk::binding(1)]] [[vk::combinedImageSampler]] Texture2D tex : register(t0);
[[vk::binding(1)]] [[vk::combinedImageSampler]] SamplerState texSampler : register(s0);
```  
If dealing with all of that is a lot, just write a macro for all of that put together. Here's one you can just copy-paste in:
```{code-block} hlsl
#define CacaoTexture(name, reg) [[vk::binding(reg)]] [[vk::combinedImageSampler]] Texture2D name : register(t##reg);\
								[[vk::binding(reg)]] [[vk::combinedImageSampler]] SamplerState name##Sampler : register(s##reg)
```  
Then you can just do this instead: `CacaoTexture(tex, 1);`  
For simplicity, maybe place this macro in a `texmacro.hlsl` file and `#include` it for ease-of-use.