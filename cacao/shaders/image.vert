#version 450 core

layout(std140,binding=0) uniform CacaoData {
    mat4 projection;

	//These two are unused but required for Cacao Engine shaders
    mat4 view;
    mat4 transform;
} cacao;

layout(location=0) in vec2 pos;
layout(location=1) in vec2 tc;

layout(location=0) out CacaoImageElem {
    vec2 texCoords;
} V2F;

void main() {
    V2F.texCoords = tc;
    gl_Position = cacao.projection * vec4(pos, 0.0, 1.0);
}