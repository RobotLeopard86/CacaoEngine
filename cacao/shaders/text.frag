#version 450 core

layout(location=0) out vec4 color;

layout(location=0) in CacaoUIQuad {
    vec2 texCoords;
	vec3 color;
} V2F;

layout(binding=0) uniform sampler2D glyph;

void main() {
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(glyph, V2F.texCoords).r);
    color = vec4(V2F.color, 1.0) * sampled;
}