#version 450 core

layout(location = 0) in vec3 position;

layout(std140, binding = 0) uniform CacaoData {
	mat4 projection;
	mat4 view;
	mat4 transform;
} cacao;

layout(location = 0) out Vertex2Fragment {
	vec4 pos;
} V2F;

void main() {
	gl_Position = cacao.projection * cacao.view * cacao.transform * vec4(position, 1.0);
	V2F.pos = gl_Position;
}