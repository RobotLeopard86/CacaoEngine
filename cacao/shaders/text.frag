#version 450 core

layout(location=0) out vec4 color;

layout(location=0) in CacaoTextElem {
    vec2 texCoords;
	vec3 color;
} V2F;

layout(binding=2) uniform sampler2D glyph;

void main() {
	vec3 textColor = pow(V2F.color, vec3(2.2));
    color = vec4(textColor, texture(glyph, V2F.texCoords).r);
	color.a = 1;
	if(color.a == 0) discard;
}