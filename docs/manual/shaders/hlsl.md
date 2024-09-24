# HLSL Shaders

Cacao Engine shaders should target Shader Model 6.0.  

## Matrix Packing
Shaders *MUST* use row-major matrix packing because the DirectX Shader Compiler changes that to column-major in SPIR-V, which is what Cacao Engine uses. See [this section](https://github.com/microsoft/DirectXShaderCompiler/blob/main/docs/SPIR-V.rst#appendix-a-matrix-representation) of the SPIR-V compatibility document for more info on why. Take a look at the whole of that document too; it will give better context as to writing HLSL for SPIR-V.  
You can either mark each matrix with the `row_major` qualifier, or add `#pragma pack_matrix(row_major)` to the top of your file.  

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
Cacao Engine sends engine data to shaders via two constant buffers. There are two ways to do this. Whichever method you use, it's reccommended to copy and paste this into your shader for best compatibility. **The order of members in these buffers is important!**  

### The `cbuffer` way
```{code-block} hlsl
cbuffer cacao_globals : register(b0) {
  float4x4 projection;
  float4x4 view;
};

cbuffer cacao_locals : register(b1) { 
	float4x4 transform;
};
```  

### The `ConstantBuffer<T>` way
```{code-block} hlsl
struct CacaoGlobals {
  float4x4 projection;
  float4x4 view;
};

struct CacaoLocals {
  float4x4 transform;
};

ConstantBuffer<CacaoGlobals> globals : register(b0);
ConstantBuffer<CacaoLocals> locals : register(b1);
```  

## Applying the Matrices
Since all of the matrices are row-major, apply them in the order below. This example assumes the output struct contains a member declared as `float4 Pos : SV_POSITION;` and the output struct is named `output`.  
```{code-block} hlsl
output.Pos = mul(pos, mul(locals.transform, mul(globals.view, globals.projection)));
```  

## Custom Shader Data
All custom shader data must be part of a struct named `ShaderData`, and must have an instance declared that is marked with the `[[vk::push_constant]]` annotation and named `shader`. Here's an example:  
```{code-block} hlsl
struct ShaderData {
  //Data goes here...
};

[[vk::push_constant]] ShaderData shader;
```  

## Textures and Samplers
In HLSL, samplers and textures are separate objects. This model is not compatible with all APIs. A `SamplerState` associated with a texture must have the same register number as that texture (e.g. `register(t0)` on the texture is connected to the sampler with `regsiter(s0)`). `SamplerState`s and textures must also all be marked with the `[[vk::combinedImageSampler]]` annotation. Here's an example:  
```{code-block} hlsl
[[vk::combinedImageSampler]] Texture2D tex : register(t0);
[[vk::combinedImageSampler]] SamplerState texSampler : register(s0);
```