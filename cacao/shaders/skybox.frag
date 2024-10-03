#version 450 core

layout(location=0) out vec4 color;

layout(location=0) in CacaoSky {
    vec3 texCoords;
} V2F;

layout(binding=2) uniform samplerCube skybox;

void main()
{
	color = texture(skybox, V2F.texCoords);
}
