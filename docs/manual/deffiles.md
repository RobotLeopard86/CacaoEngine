# Definition Files

In Cacao Engine, shaders and cubemaps are defined by definition files. These are special YAML files giving all necessary info about these assets.

## Shader Definition File Attributes
* `vertex`: Path from the working directory (set in `launchconfig.cacao.yml`) to the SPIR-V vertex shader
* `fragment`: Path from the working directory (set in `launchconfig.cacao.yml`) to the SPIR-V fragment shader
* `spec`: The shader specification, a list of shader entries.

## Shader Entry Attributes
* `name`: The name of the entry in the shader code
* `type`: The data type, one of `bool`, `int`, `uint`, `float`, or `texture`
* `sizex`: The horizontal size of the entry. For example: a GLSL `vec3` has a `sizex` value of 3, because it has three columns.
* `sizey`: The vertical size of the entry. For example: a GLSL `mat4` has a `sizey` value of 4, because it has four rows.

## Cubemap Definition File Attributes
All items are paths from the working directory (set in `launchconfig.cacao.yml`) to the desired image.
* `x+`: The image in the positive X direction (typically right)
* `x-`: The image in the negative X direction (typically left)
* `y+`: The image in the positive Y direction (typically up)
* `y-`: The image in the negative Y direction (typically down)
* `z+`: The image in the positive Z direction (typically forward)
* `z-`: The image in the negative Z direction (typically backward)  

**WARNING**: Cubemap textures should be flipped so that what you want to see at the top is at the bottom of the image. Otherwise, things will look disjointed.