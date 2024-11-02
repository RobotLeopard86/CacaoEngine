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
All GLSL vertex shaders must have the uniform `CacaoGlobals` block object, which is how engine information is passed to shaders.  
Below is an example of how this should be declared. **The order of members is important!**  
```{code-block} glsl
layout(std140,binding=0) uniform CacaoGlobals {
    mat4 projection;
    mat4 view;
} globals;
``` 

## Local Object Data
All data pertaining to an individual object (the transformation matrix plus any custom shader data) must be declared within a single push constant block, which must be declared as follows:
```{code-block} glsl
layout(push_constant) uniform ObjectData {
	mat4 transform
	//Custom shader data goes here...
} object;
```  

## Texture Bindings
In Vulkan GLSL, every uniform must have a declared `binding` value (as seen above with the uniform blocks). This includes texture samplers. They must have distinct binding values from every other binding, so you can't have a `binding=0` in your fragment shader, as that's already assigned to the `CacaoGlobals` uniform block.

## Applying the Matrices
To get the final `gl_Position` value, you should write that assignment as follows:
```{code-block} glsl
gl_Position = globals.projection * globals.view * object.transform * vec4(position, 1.0);
```

## Passing Data Between Shader Stages
Any data that is to be passed between shader stages must be declared as follows:
```{code-block} glsl
layout(location = 0) out Vertex2Fragment {
	vec4 pos;
} V2F;
```