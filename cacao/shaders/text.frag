#version 450 core

layout(location=0) out vec4 color;

layout(location=0) in CacaoTextElem {
    vec2 texCoords;
} V2F;

layout(push_constant) uniform ShaderData {
	vec3 color;
} shader;

layout(binding=2) uniform sampler2D glyph;

void main() {
	vec3 textColor = pow(shader.color, vec3(2.2));
    color = vec4(textColor, texture(glyph, V2F.texCoords).r);
	if(color.a == 0) discard;
}