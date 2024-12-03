#version 450 core

layout(location=0) out vec4 color;

layout(location=0) in CacaoImageElem {
    vec2 texCoords;
} V2F;

layout(binding=1) uniform sampler2D image;

void main() {
    vec4 sampled = texture(image, V2F.texCoords);
	color = sampled;
}