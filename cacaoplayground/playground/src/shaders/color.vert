#version 450 core

layout(location=0) in vec3 position;

layout(std140,binding=0) uniform CacaoData {
    mat4 projection;
    mat4 view;
    mat4 transform;
} cacao;

layout(location=0) out VS_Out {
	vec4 pos;
} OUT;

void main() {
    gl_Position = cacao.projection * cacao.view * cacao.transform * vec4(position, 1.0);
	OUT.pos = gl_Position;
}