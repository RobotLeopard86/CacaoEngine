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

## Cacao Engine Data
All GLSL vertex shaders must have the uniform `CacaoGlobals` block object, which is how engine information is passed to shaders.  
Below is an example of how this should be declared. **The order of members is important!**  
```{code-block} glsl
layout(std140,binding=0) uniform CacaoGlobals {
    mat4 projection;
    mat4 view;
} globals;
``` 

## Transform Matrix
All shaders must declare the transformation matrix as a push constant. This is done as follows:
```{code-block} glsl
layout(push_constant) uniform Transformation {
	mat4 transform;
};

## Local Object Data
All data for individual objects (material data) must be declared within another uniform block, which must be declared as follows:
```{code-block} glsl
layout(std140,binding=1) uniform ObjectData {
	//Material data goes here...
} object;
```  

## Texture Bindings
In Vulkan GLSL, every uniform must have a declared `binding` value (as seen above with the uniform blocks). This includes texture samplers. They must have distinct binding values from every other binding, so you can't have a `binding=0` or `binding=1` in your fragment shader, as those are already assigned to the `CacaoGlobals` and `ObjectData` uniform blocks.

## Applying the Matrices
To get the final `gl_Position` value, you should write that assignment as follows:
```{code-block} glsl
gl_Position = globals.projection * globals.view * transform * vec4(position, 1.0);
```

## Passing Data Between Shader Stages
Any data that is to be passed between shader stages must be declared as follows:
```{code-block} glsl
layout(location = 0) out Vertex2Fragment {
	vec4 pos;
} V2F;
```