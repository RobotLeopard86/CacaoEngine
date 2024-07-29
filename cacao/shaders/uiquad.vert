#version 450 core

layout(std140,binding=0) uniform CacaoData {
    mat4 projection;

	//These two are unused but required for Cacao Engine shaders
    mat4 view;
    mat4 transform;
} cacao;

layout(location=0) in vec3 pos;

layout(location=0) out CacaoUIQuad {
    vec2 texCoords;
} V2F;

void main()
{
	V2F.texCoords = pos.xy;
	gl_Position = cacao.projection * vec4(pos.xy, 0.0, 1.0);
}
