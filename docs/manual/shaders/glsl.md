# GLSL Shaders

Cacao Engine shaders must be written in Vulkan GLSL so they can be compiled to SPIR-V. They should only use `#version 450 core` or above.

## Inputs
Cacao Engine provides vertex input attributes in a specific order. Below is the code for accessing every single one, but feel free to pick and choose what you need.
```{code-block} glsl
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoords;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec3 bitangent;
layout(location = 4) in vec3 normal;
```

## Uniform Blocks
All GLSL vertex shaders must have two uniform block objects, `CacaoGlobals` and `CacaoLocals`, which are how engine information is passed to shaders.  
Below is an example of how those should be declared. **The order of members in these objects is important!**  
```{code-block} glsl
layout(std140,binding=0) uniform CacaoGlobals {
    mat4 projection;
    mat4 view;
} globals;

layout(std140,binding=1) uniform CacaoLocals {
    mat4 transform;
} locals;
```  

## Applying the Matrices
To get the final `gl_Position` value, you should write that assignment as follows:
```{code-block} glsl
gl_Position = globals.projection * globals.view * locals.transform * vec4(position, 1.0);
```

## Custom Shader Data
Any custom data must be declared within a single push constant block, which must be declared as follows:
```{code-block} glsl
layout(push_constant) uniform ShaderData {
	//Data goes here...
} shader;
```  

## Passing Data Between Shader Stages
Any data that is to be passed between shader stages must be declared as follows:
```{code-block} glsl
layout(location = 0) out Vertex2Fragment {
	vec4 pos;
} V2F;
```