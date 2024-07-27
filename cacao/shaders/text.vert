#version 450 core

layout(std140,binding=0) uniform CacaoData {
    mat4 projection;

	//These two are unused but required for Cacao shaders
    mat4 view;
    mat4 transform;
} cacao;

layout(location=0) in vec4 vertex; //(X,Y) = pos, (Z,W) = texCoords
layout(location=1) in vec3 color;

layout(location=0) out CacaoTextElem {
    vec2 texCoords;
	vec3 color;
} V2F;

void main() {
    V2F.texCoords = vertex.zw;
	V2F.color = color;
    gl_Position = cacao.projection * vec4(vertex.xy, 0.0, 1.0);
}